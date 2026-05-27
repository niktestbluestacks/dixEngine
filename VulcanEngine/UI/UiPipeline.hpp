#ifndef UI_PIPELINE_HPP
#define UI_PIPELINE_HPP

#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <memory>

namespace dix {

class UiPipeline {
public:
    UiPipeline(EngineDevice& device, vk::RenderPass renderPass);
    ~UiPipeline();

    void bind(vk::CommandBuffer commandBuffer) { if (m_pipeline) m_pipeline->bind(commandBuffer); }
    vk::PipelineLayout getLayout() const { return m_pipelineLayout; }

private:
    EngineDevice& m_device;
    std::unique_ptr<dix::Pipeline> m_pipeline;
    vk::PipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

}   // namespace dix

#endif // UI_PIPELINE_HPP