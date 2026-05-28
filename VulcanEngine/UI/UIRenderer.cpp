// dix
#include <UI/UIRenderer.hpp>
#include <Utils/Converter.hpp>

// std
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace dix {

UIRenderer::UIRenderer(EngineDevice& device, vk::RenderPass renderPass)
    : m_device(device) {
    // Descriptor layout: binding 0 = font/sprite texture sampler
    m_descriptorSetLayout =
        DixDescriptorSetLayout::Builder(m_device)
            .addBinding(0, vk::DescriptorType::eCombinedImageSampler,
                        vk::ShaderStageFlagBits::eFragment)
            .build();

    m_descriptorPool =
        DixDescriptorPool::Builder(m_device)
            .setMaxSets(10)
            .addPoolSize(vk::DescriptorType::eCombinedImageSampler, 10)
            .build();

    // Pipeline layout: one descriptor set + vec2 push constant for screen size
    vk::DescriptorSetLayout descLayout =
        m_descriptorSetLayout->getDescriptorSetLayout();
    vk::PushConstantRange pushRange{};
    pushRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
    pushRange.offset = 0;
    pushRange.size = sizeof(float) * 2;  // vec2 screenSize

    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushRange;
    if (m_device.device().createPipelineLayout(
            &layoutInfo, nullptr, &m_pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create UI pipeline layout");
    }

    // Pipeline: UI vertex layout — vec2 position, vec2 uv
    PipelineConfigInfo config{};
    Pipeline::defaultPipelineConfigInfo(config);
    config.depthStencilInfo.depthTestEnable = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;

    vk::VertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.inputRate = vk::VertexInputRate::eVertex;
    binding.stride = sizeof(float) * 4;
    config.vertexBindingDescriptions = {binding};

    vk::VertexInputAttributeDescription attrPos{};
    attrPos.binding = 0;
    attrPos.location = 0;
    attrPos.format = vk::Format::eR32G32Sfloat;
    attrPos.offset = 0;

    vk::VertexInputAttributeDescription attrUV{};
    attrUV.binding = 0;
    attrUV.location = 1;
    attrUV.format = vk::Format::eR32G32Sfloat;
    attrUV.offset = sizeof(float) * 2;
    config.vertexAttributeDescriptions = {attrPos, attrUV};

    // Alpha blending for UI elements
    config.colorBlendAttachment.blendEnable = VK_TRUE;
    config.colorBlendAttachment.srcColorBlendFactor =
        vk::BlendFactor::eSrcAlpha;
    config.colorBlendAttachment.dstColorBlendFactor =
        vk::BlendFactor::eOneMinusSrcAlpha;
    config.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    config.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    config.colorBlendAttachment.dstAlphaBlendFactor =
        vk::BlendFactor::eOneMinusSrcAlpha;
    config.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    config.pipelineLayout = m_pipelineLayout;
    config.renderPass = renderPass;

    m_pipeline =
        std::make_unique<Pipeline>(m_device, toShaderPath("UI/ui.vert.spv"),
                                   toShaderPath("UI/ui.frag.spv"), config);
}

UIRenderer::~UIRenderer() {
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        m_device.device().destroyPipelineLayout(m_pipelineLayout);
    }
}

UITexture UIRenderer::createTextureFromPixels(const unsigned char* pixels,
                                              int width, int height) {
    UITexture t;
    t.extent.width = width;
    t.extent.height = height;
    vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(width) * height * 4;

    DixBuffer staging{
        m_device, 1, static_cast<uint32_t>(imageSize),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible) |
            vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostCoherent)};
    staging.map();
    staging.writeToBuffer((void*)pixels, imageSize, 0);
    staging.flush();

    vk::ImageCreateInfo imageInfo{};
    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    m_device.createImageWithInfo(
        imageInfo,
        vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal),
        t.image, t.memory);
    m_device.copyBufferToImage(staging.getBuffer(), t.image, width, height, 1);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
    viewInfo.image = t.image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eR8G8B8A8Unorm;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (m_device.device().createImageView(&viewInfo, nullptr, &t.view) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("failed to create UI texture image view");
    }

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.sType = vk::StructureType::eSamplerCreateInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    if (m_device.device().createSampler(&samplerInfo, nullptr, &t.sampler) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("failed to create UI texture sampler");
    }

    vk::DescriptorImageInfo imageInfoDesc{};
    imageInfoDesc.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfoDesc.imageView = t.view;
    imageInfoDesc.sampler = t.sampler;

    DixDescriptorWriter(*m_descriptorSetLayout, *m_descriptorPool)
        .writeImage(0, &imageInfoDesc)
        .build(t.descriptorSet);

    return t;
}

void UIRenderer::bindPipeline(vk::CommandBuffer cb) {
    if (m_pipeline) m_pipeline->bind(cb);
}

void UIRenderer::uploadPushConstants(vk::CommandBuffer cb,
                                     vk::Extent2D screenExtent) {
    // The UI vertex shader expects a vec2 push constant: (screenWidth,
    // screenHeight).
    std::array<float, 2> screenSize{static_cast<float>(screenExtent.width),
                                    static_cast<float>(screenExtent.height)};
    cb.pushConstants(m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                     sizeof(screenSize), screenSize.data());
}

}  // namespace dix