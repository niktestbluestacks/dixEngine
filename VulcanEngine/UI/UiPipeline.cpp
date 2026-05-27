// dix
#include <UI/UiPipeline.hpp>
#include <Utils/Converter.hpp>
#include <stdexcept>

// shaders (we'll supply simple UI shaders)

namespace dix {

UiPipeline::UiPipeline(EngineDevice& device, vk::RenderPass renderPass) : m_device(device) {
    PipelineConfigInfo config{};
    Pipeline::defaultPipelineConfigInfo(config);

    // Disable depth for UI
    config.depthStencilInfo.depthTestEnable = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;

    // vertex input layout for UI vertices: vec2 pos, vec2 uv
    // Default vertex attributes will be used by Pipeline if custom ones are not provided.

    // create an empty pipeline layout (no descriptor sets) for this simple pipeline
    vk::PipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;
    if (m_device.device().createPipelineLayout(&layoutInfo, nullptr, &m_pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create ui pipeline layout");
    }

    config.pipelineLayout = m_pipelineLayout;
    config.renderPass = renderPass;

    m_pipeline = std::make_unique<Pipeline>(m_device, toShaderPath("UI/ui.vert.spv"), toShaderPath("UI/ui.frag.spv"), config);
}

UiPipeline::~UiPipeline() {
    // Pipeline destructor cleans up
}

}   // namespace dix