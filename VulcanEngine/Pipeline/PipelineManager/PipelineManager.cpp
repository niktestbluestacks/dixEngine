// dix
#include <Pipeline/PipelineManager/PipelineManager.hpp>

// std
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// ---------- EngineDevice ------------------------------------
// A thin wrapper around VkDevice that you already have in the engine.
// We just need its logical device handle.
namespace dix {

// ---------- Helpers -----------------------------------------
// Read a binary file into a byte buffer.
std::vector<char> PipelineManager::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open file: " + filename);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

// Build a shader stage from SPIR‑V code.
VkPipelineShaderStageCreateInfo PipelineManager::makeShaderStage(
        VkShaderStageFlagBits stage,
        const std::vector<char>& code,
        const std::string& entryName) {

    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize = code.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->device_.device(), &shaderModuleInfo, nullptr, &shaderModule)
        != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage  = stage;
    stageInfo.module = shaderModule;
    stageInfo.pName  = entryName.c_str();

    // We keep the module alive until pipeline creation, then destroy it.
    // (Alternatively, keep it in a map if you need to reuse it.)
    vkDestroyShaderModule(device_.logicalDevice(), shaderModule, nullptr);
    return stageInfo;
}

// ---------- Pipeline creation --------------------------------
PipelineManager::PipelineManager(EngineDevice& device) : device_(device) {
    // Nothing to do in ctor – pipelines are created on demand.
}

PipelineManager::~PipelineManager() {
    if (pipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_.logicalDevice(), pipeline_, nullptr);
    }
}

void PipelineManager::createGraphicsPipeline(const PipelineConfigInfo& cfg,
                                              const std::string& debugName) {

    // ---- 1. Create shader stages ----
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (const auto& [stage, module] : cfg.shaderStages) {
        std::vector<char> code = readFile(module.path);          // <-- load SPIR‑V
        shaderStages.push_back(makeShaderStage(stage, code));    // <-- create stage
    }

    // ---- 2. Create pipeline layout ----
    // A tiny builder to keep things readable.
    DescriptorSetLayoutBuilder setLayoutBuilder(device_);
    for (const auto& binding : cfg.descriptorSetBindings) {
        setLayoutBuilder.addBinding(binding.binding, binding.type,
                                    binding.stageFlags, binding.descriptorCount);
    }
    VkDescriptorSetLayout setLayout = setLayoutBuilder.build();

    // Push constant range (if the config has one)
    std::vector<VkPushConstantRange> pushConstantRanges;
    if (cfg.pushConstantRange.size > 0) {
        pushConstantRanges.push_back(cfg.pushConstantRange);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &setLayout;
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(device_.logicalDevice(), &pipelineLayoutInfo,
                               nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // ---- 3. Create the pipeline ----
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = &cfg.vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &cfg.inputAssemblyInfo;
    pipelineInfo.pViewportState      = &cfg.viewportStateInfo;
    pipelineInfo.pRasterizationState = &cfg.rasterizationInfo;
    pipelineInfo.pMultisampleState   = &cfg.multisampleInfo;
    pipelineInfo.pDepthStencilState  = &cfg.depthStencilInfo;
    pipelineInfo.pColorBlendState    = &cfg.colorBlendStateInfo;
    pipelineInfo.layout              = pipelineLayout;
    pipelineInfo.renderPass          = cfg.renderPass;
    pipelineInfo.subpass             = 0;  // first subpass by default
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = -1;

    if (vkCreateGraphicsPipelines(device_.logicalDevice(), VK_NULL_HANDLE,
                                   1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

}

}   // namespace dix