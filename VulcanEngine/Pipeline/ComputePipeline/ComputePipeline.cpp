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
    m_dixDevice.device().destroyShaderModule(m_compShaderModule);
    if (m_computePipeline) {
        m_dixDevice.device().destroyPipeline(m_computePipeline);
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

    assert(configInfo.pipelineLayout &&
        "Cannot create compute pipeline:: no pipelineLayout provided in configInfo");

    auto compCode = readFile(compShaderCode);

    createShaderModule(compCode, &m_compShaderModule);

    vk::PipelineShaderStageCreateInfo shaderStage{};
    shaderStage.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStage.module = m_compShaderModule;
    shaderStage.pName = "main";

    vk::ComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = configInfo.pipelineLayout;

    auto result = m_dixDevice.device().createComputePipelines({}, {pipelineInfo});
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create compute pipeline");
    }
    m_computePipeline = result.value.front();

}

void ComputePipeline::createShaderModule(const std::vector<char>& code, vk::ShaderModule* shaderModule) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    auto result = m_dixDevice.device().createShaderModule(createInfo);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module!");
    }
    *shaderModule = result.value;
}

void ComputePipeline::bind(vk::CommandBuffer commandBuffer) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_computePipeline);
}

}   // namespace dix