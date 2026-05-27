// dix
#include <Logger/Logger.hpp>
#include <Model/Model.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>


// std
#include <cassert>
#include <fstream>
#include <stdexcept>


namespace dix {

Pipeline::Pipeline(EngineDevice& device, const std::string& vertFilepath,
                   const std::string& fragFilepath,
                   const PipelineConfigInfo& configInfo)
    : dixdevice{device} {
    createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

Pipeline::~Pipeline() {
    dixdevice.device().destroyShaderModule(vertShaderModule, nullptr);
    dixdevice.device().destroyShaderModule(fragShaderModule, nullptr);
    if (graphicsPipeline) {
        dixdevice.device().destroyPipeline(graphicsPipeline, nullptr);
    }
}

std::vector<char> Pipeline::readFile(const std::string& filepath) {
    std::ifstream file{filepath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void Pipeline::createGraphicsPipeline(const std::string& vertShaderCode,
                                      const std::string& fragShaderCode,
                                      const PipelineConfigInfo& configInfo) {
    assert(configInfo.pipelineLayout &&
           "Cannot create graphic pipeline:: no pipelineLayout provided in "
           "configInfo");

    assert(configInfo.renderPass &&
           "Cannot create graphic pipeline:: no renderPass provided in "
           "configInfo");

    auto vertCode = readFile(vertShaderCode);
    auto fragCode = readFile(fragShaderCode);

    createShaderModule(vertCode, &vertShaderModule);
    createShaderModule(fragCode, &fragShaderModule);

    vk::PipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = vk::PipelineShaderStageCreateFlags{};
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = vk::PipelineShaderStageCreateFlags{};
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        vk::StructureType::ePipelineVertexInputStateCreateInfo;
    // allow custom vertex input descriptions via configInfo; otherwise use
    // Model::Vertex
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    if (!configInfo.vertexBindingDescriptions.empty() &&
        !configInfo.vertexAttributeDescriptions.empty()) {
        bindingDescriptions = configInfo.vertexBindingDescriptions;
        attributeDescriptions = configInfo.vertexAttributeDescriptions;
    } else {
        bindingDescriptions = Model::Vertex::getBindingDescriptions();
        attributeDescriptions = Model::Vertex::getAttributeDescriptions();
    }
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizetionInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleStateInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = nullptr;

    auto result = dixdevice.device().createGraphicsPipelines(
        nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline");
    }
}

void Pipeline::createShaderModule(const std::vector<char>& code,
                                  vk::ShaderModule* shaderModule) {
    vk::ShaderModuleCreateInfo createInfo{};

    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    auto result = dixdevice.device().createShaderModule(&createInfo, nullptr,
                                                        shaderModule);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module!");
    }
}

void Pipeline::bind(vk::CommandBuffer commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               graphicsPipeline);
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
    configInfo.inputAssemblyInfo.sType =
        vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    configInfo.inputAssemblyInfo.topology =
        vk::PrimitiveTopology::eTriangleList;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewportInfo.sType =
        vk::StructureType::ePipelineViewportStateCreateInfo;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

    configInfo.rasterizetionInfo.sType =
        vk::StructureType::ePipelineRasterizationStateCreateInfo;
    configInfo.rasterizetionInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizetionInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizetionInfo.polygonMode = vk::PolygonMode::eFill;
    configInfo.rasterizetionInfo.lineWidth = 1.0f;
    configInfo.rasterizetionInfo.cullMode = vk::CullModeFlagBits::eNone;
    configInfo.rasterizetionInfo.frontFace = vk::FrontFace::eClockwise;
    configInfo.rasterizetionInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizetionInfo.depthBiasConstantFactor = 0.0f;  // optional
    configInfo.rasterizetionInfo.depthBiasClamp = 0.0f;           // optional
    configInfo.rasterizetionInfo.depthBiasSlopeFactor = 0.0f;     // optional

    configInfo.multisampleStateInfo.sType =
        vk::StructureType::ePipelineMultisampleStateCreateInfo;
    configInfo.multisampleStateInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleStateInfo.rasterizationSamples =
        vk::SampleCountFlagBits::e1;
    configInfo.multisampleStateInfo.minSampleShading = 1.0f;  // optional
    configInfo.multisampleStateInfo.pSampleMask = nullptr;    // optional
    configInfo.multisampleStateInfo.alphaToCoverageEnable =
        VK_FALSE;                                                 // optional
    configInfo.multisampleStateInfo.alphaToOneEnable = VK_FALSE;  // optional

    configInfo.colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor =
        vk::BlendFactor::eOne;  // optional
    configInfo.colorBlendAttachment.dstColorBlendFactor =
        vk::BlendFactor::eZero;  // optional
    configInfo.colorBlendAttachment.colorBlendOp =
        vk::BlendOp::eAdd;  // optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor =
        vk::BlendFactor::eOne;  // optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor =
        vk::BlendFactor::eZero;  // optional
    configInfo.colorBlendAttachment.alphaBlendOp =
        vk::BlendOp::eAdd;  // optional

    configInfo.colorBlendInfo.sType =
        vk::StructureType::ePipelineColorBlendStateCreateInfo;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy;  // optional
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // optional

    configInfo.depthStencilInfo.sType =
        vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = vk::StencilOpState{};  // optional
    configInfo.depthStencilInfo.back = vk::StencilOpState{};   // optional

    configInfo.dynamicStateEnables = {vk::DynamicState::eViewport,
                                      vk::DynamicState::eScissor};
    configInfo.dynamicStateInfo.sType =
        vk::StructureType::ePipelineDynamicStateCreateInfo;
    configInfo.dynamicStateInfo.pDynamicStates =
        configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount =
        static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags = vk::PipelineDynamicStateCreateFlags{};
}
}  // namespace dix