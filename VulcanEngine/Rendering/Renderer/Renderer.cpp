// dix
#include <Rendering/Renderer/Renderer.hpp>
#include <Utils/Converter.hpp>

// std
#include <stdexcept>
#include <cstdint>
#include <array>

namespace dix {

Renderer::Renderer(Window& window, EngineDevice& engineDevice) :
		m_Window{window},
		m_dixDevice{engineDevice} {

	recreateSwapChain();
	createCommandBuffers();
}

Renderer::~Renderer() {
	freeCommandBuffers();
}

void Renderer::recreateSwapChain(void) {
	auto extent = m_Window.getExtent();
	while (extent.width == 0 || extent.height == 0) {
		extent = m_Window.getExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_dixDevice.device());
	if (m_dixSwapChain == nullptr) {
		m_dixSwapChain = std::make_unique <SwapChain>(m_dixDevice, extent);
	}
	else {
		std::shared_ptr <SwapChain>	oldSwapChain = std::move(m_dixSwapChain);
		m_dixSwapChain = std::make_unique <SwapChain>(m_dixDevice, extent, oldSwapChain);

		if (!oldSwapChain->compareSwapFormats(*m_dixSwapChain.get())) {
			throw std::runtime_error("Swap chain image / depth format has changed!");
		}
	}

	// TODO: if render pass is compatable do nothing else recreate
}

void Renderer::createCommandBuffers(void) {

	m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_dixDevice.getCommandPool();
	allocInfo.commandBufferCount = static_cast <uint32_t> (m_commandBuffers.size());

	if (vkAllocateCommandBuffers(
		m_dixDevice.device(),
		&allocInfo,
		m_commandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Renderer::freeCommandBuffers(void) {
	vkFreeCommandBuffers(
		m_dixDevice.device(),
		m_dixDevice.getCommandPool(),
		static_cast <uint32_t> (m_commandBuffers.size()),
		m_commandBuffers.data());

	m_commandBuffers.clear();
}

int Renderer::getFrameIndex(void) const {
	assert(m_isFrameStarted && "Can't call GetFrameIndex when frame is not in progress");
	return m_currentFrameIndex;
}

VkCommandBuffer Renderer::beginFrame(void) {
	assert(!m_isFrameStarted && "Can't call begin frame while already in progress");
	auto result = m_dixSwapChain->acquireNextImage(&m_currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	m_isFrameStarted = true;

	auto commandBuffer = getCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(
		commandBuffer,
		&beginInfo) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	return commandBuffer;

}

void Renderer::endFrame(void) {
	assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
	auto commandBuffer = getCurrentCommandBuffer();

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

    auto result = m_dixSwapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR ||
		m_Window.wasWindowResized()) {
		m_Window.resetWindowResizedFlag();
		recreateSwapChain();
	}
    else if (result != VK_SUCCESS) {
		// try to recover by recreating the swapchain for any non-success result
		m_Window.resetWindowResizedFlag();
		recreateSwapChain();
	}

	m_isFrameStarted = false;
	m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
	assert(m_isFrameStarted && "Can't begin swap chain render pass (call beginSwapChainRenderPass) if frame is not in progress");
	assert(commandBuffer == (getCurrentCommandBuffer()) && "Can't begin render pass on a command buffer from a different frame");


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_dixSwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_dixSwapChain->getFrameBuffer(m_currentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_dixSwapChain->getSwapChainExtent();

	std::array <VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast <uint32_t> (clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast <float> (m_dixSwapChain->getSwapChainExtent().width);
	viewport.height = static_cast <float> (m_dixSwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_dixSwapChain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
	assert(
		m_isFrameStarted && 
		"Can't end swap chain render pass (call endSwapChainRenderPass) if frame is not in progress");
	assert(
		commandBuffer == (getCurrentCommandBuffer()) && 
		"Can't end render pass on a command buffer from a different frame");

	vkCmdEndRenderPass(commandBuffer);
}

}	// namespace dix