#ifndef _PIPELINE_CONFIG_INFO_HPP_
#define _PIPELINE_CONFIG_INFO_HPP_

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/ShaderModule/ShaderModule.hpp>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    vk::PipelineViewportStateCreateInfo viewportInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo{};
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
    std::vector<vk::DynamicState> dynamicStateEnables;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::RenderPass renderPass = nullptr;
    uint32_t subpass = 0;

    // Optional custom vertex input descriptions
    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
};

}  // namespace dix

#endif // _PIPELINE_CONFIG_INFO_HPP_