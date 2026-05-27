// dix
#include <Pipeline/ComputePipeline/ComputePipeline.hpp>
#include <Logger/Logger.hpp>
#include <Utils/Converter.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// std
#include <stdexcept>
#include <cassert>
#include <fstream>

namespace dix {
ComputePipeline::ComputePipeline(
    EngineDevice& device,
    const std::string& compFilepath,
    const ComputePipelineConfigInfo& configInfo
) :
    m_dixDevice{device} {

    createComputePipeline(compFilepath, configInfo);
}

ComputePipeline::~ComputePipeline() {
    vkDestroyShaderModule(m_dixDevice.device(), m_compShaderModule, nullptr);
    if (m_computePipeline) {
        vkDestroyPipeline(m_dixDevice.device(), m_computePipeline, nullptr);
    }
}

std::vector<char> ComputePipeline::readFile(const std::string& filepath) {

    std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

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

void ComputePipeline::createComputePipeline(
    const std::string& compShaderCode,
    const ComputePipelineConfigInfo& configInfo
) {

    assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
        "Cannot create compute pipeline:: no pipelineLayout provided in configInfo");

    auto compCode = readFile(compShaderCode);

    createShaderModule(compCode, &m_compShaderModule);

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStage.module = m_compShaderModule;
    shaderStage.pName = "main";
    shaderStage.flags = 0;
    shaderStage.pNext = nullptr;
    shaderStage.pSpecializationInfo = nullptr;

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateComputePipelines(
        m_dixDevice.device(),
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &m_computePipeline) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create compute pipeline");
    }

}

void ComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
    VkShaderModuleCreateInfo createInfo {};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(
            m_dixDevice.device(), 
            &createInfo, 
            nullptr, 
            shaderModule
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create shader module!");
    }
}

void ComputePipeline::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
}

}   // namespace dix