#pragma once

#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Rendering/Renderer/Renderer.hpp>

#include <string>
#include <vector>
#include <memory>

namespace dix {

struct UITexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

class UIRenderer {
public:
    UIRenderer(EngineDevice& device, VkRenderPass renderPass);
    ~UIRenderer();

    // create texture from raw RGBA pixels
    UITexture createTextureFromPixels(const unsigned char* pixels, int width, int height);

    // helpers
    void bindPipeline(VkCommandBuffer cb);
    VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    DixDescriptorPool& getDescriptorPool() { return *m_descriptorPool; }
    DixDescriptorSetLayout& getDescriptorSetLayout() { return *m_descriptorSetLayout; }

private:
    EngineDevice& m_device;
    std::unique_ptr<DixDescriptorSetLayout> m_descriptorSetLayout;
    std::unique_ptr<DixDescriptorPool> m_descriptorPool;
    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};

}