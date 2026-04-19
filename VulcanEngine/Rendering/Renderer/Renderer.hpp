#ifndef RENDERER_HPP
#define RENDERER_HPP
// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/SwapChain/SwapChain.hpp>
#include <Window/WindowClass/WindowClass.hpp>
#include <Logger/Logger.hpp>

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

	VkRenderPass getSwapChainRenderPass() const { return m_dixSwapChain->getRenderPass(); }

	bool isFrameInProgress() const { return m_isFrameStarted; }

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(m_isFrameStarted && "Cannot get command buffer when frame is not in progress");
		return  m_commandBuffers[m_currentImageIndex];
	}

	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
private:
	Window& m_Window;
	EngineDevice& m_dixDevice;
	std::unique_ptr <SwapChain> m_dixSwapChain;
	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32_t m_currentImageIndex;
	bool m_isFrameStarted;
};	// class Renderer
}	// namespace dix
#endif // RENDERER_HPP
