// dix
#include <Pipeline/PipelineConfigInfo/PipelineConfigInfo.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>

namespace dix {

VkPipelineVertexInputStateCreateInfo createVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) {
    VkPipelineVertexInputStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    createInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    createInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    return createInfo;
}

VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.topology = topology;

    return createInfo;
}

std::pair<VkViewport, VkRect2D> createViewportAndScissor(int width, int height) {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    return std::make_pair(viewport, scissor);
}

VkPipelineRasterizationStateCreateInfo createRasterizationState(VkPolygonMode polygonMode, bool cullModeBack) {
    VkPipelineRasterizationStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.depthClampEnable = VK_FALSE;
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    createInfo.polygonMode = polygonMode;
    createInfo.lineWidth = 1.0f;
    createInfo.cullMode = cullModeBack ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    createInfo.depthBiasEnable = VK_FALSE;

    return createInfo;
}

VkPipelineMultisampleStateCreateInfo createMultisampleState() {
    VkPipelineMultisampleStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.sampleShadingEnable = VK_FALSE;
    createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    return createInfo;
}

VkPipelineDepthStencilStateCreateInfo createDepthStencilState(bool enableDepthTesting, bool enableDepthWriting, VkCompareOp compareOp) {
    VkPipelineDepthStencilStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.depthTestEnable = enableDepthTesting ? VK_TRUE : VK_FALSE;
    createInfo.depthWriteEnable = enableDepthWriting ? VK_TRUE : VK_FALSE;
    createInfo.depthCompareOp = compareOp;
    createInfo.minDepthBounds = 0.0f; // Optional
    createInfo.maxDepthBounds = 1.0f; // Optional
    createInfo.stencilTestEnable = VK_FALSE;

    return createInfo;
}

VkPipelineColorBlendAttachmentState createColorBlendAttachmentState(bool blendEnable) {
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = blendEnable ? VK_TRUE : VK_FALSE;

    if (blendEnable) {
        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
        attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    } else {
        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
        attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    return attachment;
}

VkPipelineColorBlendStateCreateInfo createColorBlendState(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
    VkPipelineColorBlendStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.logicOpEnable = VK_FALSE;
    createInfo.logicOp = VK_LOGIC_OP_COPY;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.blendConstants[0] = 0.0f;
    createInfo.blendConstants[1] = 0.0f;
    createInfo.blendConstants[2] = 0.0f;
    createInfo.blendConstants[3] = 0.0f;

    return createInfo;
}
}   // namespace dix