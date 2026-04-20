#include "UiPipeline.hpp"
#include <Utils/Converter.hpp>

// shaders (we'll supply simple UI shaders)

namespace dix {

UiPipeline::UiPipeline(EngineDevice& device, VkRenderPass renderPass) : m_device(device) {
    PipelineConfigInfo config{};
    Pipeline::defaultPipelineConfigInfo(config);

    // Disable depth for UI
    config.depthStencilInfo.depthTestEnable = VK_FALSE;
    config.depthStencilInfo.depthWriteEnable = VK_FALSE;

    // vertex input layout for UI vertices: vec2 pos, vec2 uv
    // Default vertex attributes will be used by Pipeline if custom ones are not provided.

    config.pipelineLayout = m_pipelineLayout; // null for now; pipeline will create its own if needed
    config.renderPass = renderPass;

    m_pipeline = std::make_unique<Pipeline>(m_device, toShaderPath("UI/ui.vert.spv"), toShaderPath("UI/ui.frag.spv"), config);
}

UiPipeline::~UiPipeline() {
    // Pipeline destructor cleans up
}

}
