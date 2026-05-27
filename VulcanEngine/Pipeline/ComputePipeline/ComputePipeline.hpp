#ifndef COMPUTE_PIPELINE_HPP
#define COMPUTE_PIPELINE_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/ShaderModule/ShaderModule.hpp>
#include <Utils/Class.hpp>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

struct ComputePipelineConfigInfo {
    ComputePipelineConfigInfo() = default;
    ComputePipelineConfigInfo(vk::PipelineLayout lay) : pipelineLayout(lay) {}
    DIX_DISABLE_COPY(ComputePipelineConfigInfo)

    vk::PipelineLayout pipelineLayout;
};

class ComputePipeline {
public:
    ComputePipeline(
        EngineDevice& device,
        const std::string& compFilepath,
        const ComputePipelineConfigInfo& configInfo);

    ~ComputePipeline();

    DIX_DISABLE_COPY(ComputePipeline)

    void bind(vk::CommandBuffer commandBuffer);

    vk::Pipeline& getPipeline() {
        return m_computePipeline;
    }
private:
    static std::vector <char> readFile(const std::string& filepath);

    void createComputePipeline(
        const std::string& compShaderCode,
        const ComputePipelineConfigInfo& configInfo);

    void createShaderModule(const std::vector<char>& code, vk::ShaderModule* shaderModule);

private:

    EngineDevice& m_dixDevice;
    vk::Pipeline m_computePipeline;
    vk::ShaderModule m_compShaderModule;
};

}   // namespace dix

#endif // COMPUTE_PIPELINE_HPP