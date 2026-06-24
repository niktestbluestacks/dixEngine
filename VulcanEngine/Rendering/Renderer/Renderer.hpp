#ifndef RENDERER_HPP
#define RENDERER_HPP

// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/SwapChain/SwapChain.hpp>
#include <Window/WindowClass/WindowClass.hpp>

// std
#include <cassert>
#include <memory>
#include <vector>

namespace dix {
class Renderer {
   private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

   public:
    Renderer(Window& window, EngineDevice& engineDevice);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    vk::RenderPass getSwapChainRenderPass() const {
        return m_dixSwapChain->getRenderPass();
    }
    float getAspectRatio() const { return m_dixSwapChain->extentAspectRatio(); }
    bool isFrameInProgress() const { return m_isFrameStarted; }

    vk::CommandBuffer getCurrentCommandBuffer() const {
        assert(m_isFrameStarted &&
               "Cannot get command buffer when frame is not in progress");
        return m_commandBuffers[m_currentFrameIndex];
    }

    int getFrameIndex() const;

    vk::CommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
    void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

   private:
    Window& m_Window;
    EngineDevice& m_dixDevice;
    std::unique_ptr<SwapChain> m_dixSwapChain;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    uint32_t m_currentImageIndex = 0;
    int m_currentFrameIndex = 0;
    bool m_isFrameStarted = false;
};  // class Renderer
}  // namespace dix
#endif  // RENDERER_HPP