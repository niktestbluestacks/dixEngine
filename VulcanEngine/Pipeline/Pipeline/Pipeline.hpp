#ifndef PIPELINE_HPP
#define PIPELINE_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <cstdint>
#include <string>
#include <vector>

namespace dix {

struct PipelineConfigInfo {
    PipelineConfigInfo() {
        this->inputAssemblyInfo.sType =
            vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        this->inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        this->inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        this->viewportInfo.sType =
            vk::StructureType::ePipelineViewportStateCreateInfo;
        this->viewportInfo.viewportCount = 1;
        this->viewportInfo.pViewports = nullptr;
        this->viewportInfo.scissorCount = 1;
        this->viewportInfo.pScissors = nullptr;

        this->rasterizetionInfo.sType =
            vk::StructureType::ePipelineRasterizationStateCreateInfo;
        this->rasterizetionInfo.depthClampEnable = VK_FALSE;
        this->rasterizetionInfo.rasterizerDiscardEnable = VK_FALSE;
        this->rasterizetionInfo.polygonMode = vk::PolygonMode::eFill;
        this->rasterizetionInfo.lineWidth = 1.0f;
        this->rasterizetionInfo.cullMode = vk::CullModeFlagBits::eNone;
        this->rasterizetionInfo.frontFace = vk::FrontFace::eClockwise;
        this->rasterizetionInfo.depthBiasEnable = VK_FALSE;
        this->rasterizetionInfo.depthBiasConstantFactor = 0.0f;  // optional
        this->rasterizetionInfo.depthBiasClamp = 0.0f;           // optional
        this->rasterizetionInfo.depthBiasSlopeFactor = 0.0f;     // optional

        this->multisampleStateInfo.sType =
            vk::StructureType::ePipelineMultisampleStateCreateInfo;
        this->multisampleStateInfo.sampleShadingEnable = VK_FALSE;
        this->multisampleStateInfo.rasterizationSamples =
            vk::SampleCountFlagBits::e1;
        this->multisampleStateInfo.minSampleShading = 1.0f;  // optional
        this->multisampleStateInfo.pSampleMask = nullptr;    // optional
        this->multisampleStateInfo.alphaToCoverageEnable =
            VK_FALSE;                                            // optional
        this->multisampleStateInfo.alphaToOneEnable = VK_FALSE;  // optional

        this->colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        this->colorBlendAttachment.blendEnable = VK_FALSE;
        this->colorBlendAttachment.srcColorBlendFactor =
            vk::BlendFactor::eOne;  // optional
        this->colorBlendAttachment.dstColorBlendFactor =
            vk::BlendFactor::eZero;  // optional
        this->colorBlendAttachment.colorBlendOp =
            vk::BlendOp::eAdd;  // optional
        this->colorBlendAttachment.srcAlphaBlendFactor =
            vk::BlendFactor::eOne;  // optional
        this->colorBlendAttachment.dstAlphaBlendFactor =
            vk::BlendFactor::eZero;  // optional
        this->colorBlendAttachment.alphaBlendOp =
            vk::BlendOp::eAdd;  // optional

        this->colorBlendInfo.sType =
            vk::StructureType::ePipelineColorBlendStateCreateInfo;
        this->colorBlendInfo.logicOpEnable = VK_FALSE;
        this->colorBlendInfo.logicOp = vk::LogicOp::eCopy;  // optional
        this->colorBlendInfo.attachmentCount = 1;
        this->colorBlendInfo.pAttachments = &this->colorBlendAttachment;
        this->colorBlendInfo.blendConstants[0] = 0.0f;  // optional
        this->colorBlendInfo.blendConstants[1] = 0.0f;  // optional
        this->colorBlendInfo.blendConstants[2] = 0.0f;  // optional
        this->colorBlendInfo.blendConstants[3] = 0.0f;  // optional

        this->depthStencilInfo.sType =
            vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        this->depthStencilInfo.depthTestEnable = VK_TRUE;
        this->depthStencilInfo.depthWriteEnable = VK_TRUE;
        this->depthStencilInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;
        this->depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        this->depthStencilInfo.minDepthBounds = 0.0f;  // optional
        this->depthStencilInfo.maxDepthBounds = 1.0f;  // optional
        this->depthStencilInfo.stencilTestEnable = VK_FALSE;
        this->depthStencilInfo.front = vk::StencilOpState{};  // optional
        this->depthStencilInfo.back = vk::StencilOpState{};   // optional

        this->dynamicStateEnables = {vk::DynamicState::eViewport,
                                     vk::DynamicState::eScissor};
        this->dynamicStateInfo.sType =
            vk::StructureType::ePipelineDynamicStateCreateInfo;
        this->dynamicStateInfo.pDynamicStates =
            this->dynamicStateEnables.data();
        this->dynamicStateInfo.dynamicStateCount =
            static_cast<uint32_t>(this->dynamicStateEnables.size());
        this->dynamicStateInfo.flags = vk::PipelineDynamicStateCreateFlags{};
    }

    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo(PipelineConfigInfo&&) = default;
    PipelineConfigInfo& operator=(PipelineConfigInfo&&) = default;

    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    vk::PipelineRasterizationStateCreateInfo rasterizetionInfo;
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<vk::DynamicState> dynamicStateEnables;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::RenderPass renderPass = nullptr;
    uint32_t subpass = 0;

    // optional custom vertex input descriptions
    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription>
        vertexAttributeDescriptions;
};

class Pipeline {
   public:
    Pipeline(EngineDevice& device, const std::string& vertFilepath,
             const std::string& fragFilepath,
             const PipelineConfigInfo& configInfo);

    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline operator=(const Pipeline&) = delete;

    void bind(vk::CommandBuffer commandBuffer);
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    vk::Pipeline& getPipeline() { return graphicsPipeline; }

   private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertShaderCode,
                                const std::string& fragShaderCode,
                                const PipelineConfigInfo& pipelineInfo);

    void createShaderModule(const std::vector<char>& code,
                            vk::ShaderModule* shaderModule);

   private:
    EngineDevice& dixdevice;
    vk::Pipeline graphicsPipeline;
    vk::ShaderModule vertShaderModule;
    vk::ShaderModule fragShaderModule;
};  // class Pipeline
}  // namespace dix

#endif  // PIPELINE_HPP