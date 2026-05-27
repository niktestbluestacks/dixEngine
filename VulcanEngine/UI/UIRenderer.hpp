#ifndef UI_RENDERER_HPP
#define UI_RENDERER_HPP

// dix
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Rendering/Renderer/Renderer.hpp>


// std
#include <memory>

namespace dix {

struct UITexture {
    vk::Image image = VK_NULL_HANDLE;
    vk::DeviceMemory memory = VK_NULL_HANDLE;
    vk::ImageView view = VK_NULL_HANDLE;
    vk::Sampler sampler = VK_NULL_HANDLE;
    vk::DescriptorSet descriptorSet = VK_NULL_HANDLE;
    vk::Extent2D extent{};
};

class UIRenderer {
   public:
    UIRenderer(EngineDevice& device, vk::RenderPass renderPass);
    ~UIRenderer();

    // Creates a texture from raw RGBA pixels (e.g. for font atlases).
    UITexture createTextureFromPixels(const unsigned char* pixels, int width,
                                      int height);

    // Binds the UI pipeline.  Must be called inside a render pass.
    void bindPipeline(vk::CommandBuffer cb);

    // Uploads the screen-size push constants required by the UI vertex shader.
    // Call after bindPipeline and before issuing any UI draw calls.
    void uploadPushConstants(vk::CommandBuffer cb, vk::Extent2D screenExtent);

    vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    DixDescriptorPool& getDescriptorPool() { return *m_descriptorPool; }
    DixDescriptorSetLayout& getDescriptorSetLayout() {
        return *m_descriptorSetLayout;
    }
    EngineDevice& getDevice() { return m_device; }

   private:
    EngineDevice& m_device;
    std::unique_ptr<DixDescriptorSetLayout> m_descriptorSetLayout;
    std::unique_ptr<DixDescriptorPool> m_descriptorPool;
    std::unique_ptr<Pipeline> m_pipeline;
    vk::PipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

}  // namespace dix
#endif  // UI_RENDERER_HPP