// dix
#include <UI/UIRenderer.hpp>
#include <Utils/Converter.hpp>

// std
#include <stdexcept>
#include <string>
#include <vector>
#include <array>

namespace dix {

UIRenderer::UIRenderer(EngineDevice& device, VkRenderPass renderPass) : m_device(device) {
    // Descriptor layout: binding 0 = font/sprite texture sampler
    m_descriptorSetLayout = DixDescriptorSetLayout::Builder(m_device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    m_descriptorPool = DixDescriptorPool::Builder(m_device)
        .setMaxSets(10)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
        .build();

    // Pipeline layout: one descriptor set + vec2 push constant for screen size
    VkDescriptorSetLayout descLayout = m_descriptorSetLayout->getDescriptorSetLayout();
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset     = 0;
    pushRange.size       = sizeof(float) * 2; // vec2 screenSize

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount         = 1;
    layoutInfo.pSetLayouts            = &descLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges    = &pushRange;
    if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create UI pipeline layout");
    }

    // Pipeline: UI vertex layout — vec2 position, vec2 uv
    PipelineConfigInfo config{};
    Pipeline::defaultPipelineConfigInfo(config);
    config.depthStencilInfo.depthTestEnable  = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;

    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.stride    = sizeof(float) * 4;
    config.vertexBindingDescriptions = { binding };

    VkVertexInputAttributeDescription attrPos{};
    attrPos.binding  = 0; attrPos.location = 0;
    attrPos.format   = VK_FORMAT_R32G32_SFLOAT; attrPos.offset = 0;

    VkVertexInputAttributeDescription attrUV{};
    attrUV.binding   = 0; attrUV.location = 1;
    attrUV.format    = VK_FORMAT_R32G32_SFLOAT; attrUV.offset = sizeof(float) * 2;
    config.vertexAttributeDescriptions = { attrPos, attrUV };

    // Alpha blending for UI elements
    config.colorBlendAttachment.blendEnable         = VK_TRUE;
    config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    config.colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    config.colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    config.pipelineLayout = m_pipelineLayout;
    config.renderPass     = renderPass;

    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        toShaderPath("UI/ui.vert.spv"),
        toShaderPath("UI/ui.frag.spv"),
        config);
}

UIRenderer::~UIRenderer() {
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }
}

UITexture UIRenderer::createTextureFromPixels(const unsigned char* pixels, int width, int height) {
    UITexture t;
    t.extent.width  = width;
    t.extent.height = height;
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * 4;

    DixBuffer staging{
        m_device,
        1,
        static_cast<uint32_t>(imageSize),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    staging.map();
    staging.writeToBuffer((void*)pixels, imageSize, 0);
    staging.flush();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, t.image, t.memory);
    m_device.copyBufferToImage(staging.getBuffer(), t.image, width, height, 1);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = t.image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &t.view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create UI texture image view");
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter        = VK_FILTER_LINEAR;
    samplerInfo.minFilter        = VK_FILTER_LINEAR;
    samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy    = 1.0f;
    samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareEnable    = VK_FALSE;
    samplerInfo.compareOp        = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &t.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create UI texture sampler");
    }

    VkDescriptorImageInfo imageInfoDesc{};
    imageInfoDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoDesc.imageView   = t.view;
    imageInfoDesc.sampler     = t.sampler;

    DixDescriptorWriter(*m_descriptorSetLayout, *m_descriptorPool)
        .writeImage(0, &imageInfoDesc)
        .build(t.descriptorSet);

    return t;
}

void UIRenderer::bindPipeline(VkCommandBuffer cb) {
    if (m_pipeline) m_pipeline->bind(cb);
}

void UIRenderer::uploadPushConstants(VkCommandBuffer cb, VkExtent2D screenExtent) {
    // The UI vertex shader expects a vec2 push constant: (screenWidth, screenHeight).
    std::array<float, 2> screenSize{
        static_cast<float>(screenExtent.width),
        static_cast<float>(screenExtent.height)
    };
    vkCmdPushConstants(
        cb,
        m_pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(screenSize),
        screenSize.data());
}

}   // namespace dix
