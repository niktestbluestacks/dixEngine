#include <FirstApp/FirstApp.hpp>
#include <Utils/Converter.hpp>

// std
#include <stdexcept>
#include <cstdint>
#include <array>

namespace dix {
FirstApp::FirstApp() {
	Logger::get().log(Logger::INFO, "Initiating the application...");
	Logger::get().log(Logger::INFO, "-----------------------------");
	loadModels();
	createPipelineLayout();
	recreateSwapChain();
	createCommandBuffers();
	Logger::get().log(Logger::INFO, "Application initialized succsesfully!");
}

FirstApp::~FirstApp() {
	Logger::get().log(Logger::INFO, "Closing the application...");
	Logger::get().log(Logger::INFO, "--------------------------");
	vkDestroyPipelineLayout(m_dixDevice.device(), m_pipelineLayout, nullptr);
	Logger::get().log(Logger::INFO, "Application has been closed succsesfully!");
}

void FirstApp::run(void) {
	while (!m_Window.shouldClose()) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(m_dixDevice.device());
}

void FirstApp::sierpinski(
		std::vector<Model::Vertex>& vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top,
		glm::vec3 leftColor,
		glm::vec3 rightColor,
		glm::vec3 topColor) {
	if (depth <= 0) {
		vertices.push_back({ top, topColor });
		vertices.push_back({ right, rightColor });
		vertices.push_back({ left, leftColor });
	}
	else {
		auto leftTop = 0.5f * (left + top);
		auto rightTop = 0.5f * (right + top);
		auto leftRight = 0.5f * (left + right);

		auto leftTopColor = 0.5f * (leftColor + topColor);
		auto rightTopColor = 0.5f * (rightColor + topColor);
		auto leftRightColor = 0.5f * (leftColor + rightColor);

		sierpinski(vertices, depth - 1, left, leftRight, leftTop, leftColor, leftRightColor, leftTopColor);
		sierpinski(vertices, depth - 1, leftRight, right, rightTop, leftRightColor, rightColor, rightTopColor);
		sierpinski(vertices, depth - 1, leftTop, rightTop, top, leftTopColor, rightTopColor, topColor);
	}
}

void FirstApp::loadModels() {
	std::vector <Model::Vertex> vertices{};

	sierpinski(vertices, 8, { 0.0f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

	m_dixModel = std::make_unique <Model>(m_dixDevice, vertices);
}

void FirstApp::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(m_dixDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != 
			VK_SUCCESS) {
		throw std::runtime_error("failed to create pipelineLayout");
	}
}

void FirstApp::createPipeline() {
	assert(m_dixSwapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = m_dixSwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique <Pipeline>(
		m_dixDevice,
		toShaderPath("SimpleShader/simple_shader.vert.spv"),
		toShaderPath("SimpleShader/simple_shader.frag.spv"),
		pipelineConfig);
}

void FirstApp::recreateSwapChain() {
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
		m_dixSwapChain = std::make_unique <SwapChain>(m_dixDevice, extent, std::move(m_dixSwapChain));
		if (m_dixSwapChain->imageCount() != m_commandBuffers.size()) {
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	// TODO: if render pass is compatable do nothing else recreate
	createPipeline();
}

void FirstApp::createCommandBuffers() {

	m_commandBuffers.resize(m_dixSwapChain->imageCount());

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

void FirstApp::freeCommandBuffers() {
	vkFreeCommandBuffers(
		m_dixDevice.device(), 
		m_dixDevice.getCommandPool(), 
		static_cast <uint32_t> (m_commandBuffers.size()), 
		m_commandBuffers.data());

	m_commandBuffers.clear();
}

void FirstApp::recordCommandBuffer(int imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(
		m_commandBuffers[imageIndex],
		&beginInfo) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_dixSwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_dixSwapChain->getFrameBuffer(imageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_dixSwapChain->getSwapChainExtent();

	std::array <VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast <uint32_t> (clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast <float> (m_dixSwapChain->getSwapChainExtent().width);
	viewport.height = static_cast <float> (m_dixSwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_dixSwapChain->getSwapChainExtent() };
	vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);


	m_pipeline->bind(m_commandBuffers[imageIndex]);
	m_dixModel->bind(m_commandBuffers[imageIndex]);
	m_dixModel->draw(m_commandBuffers[imageIndex]);
	vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
	if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void FirstApp::drawFrame() {
	uint32_t imageIndex;
	auto result = m_dixSwapChain->acquireNextImage(&imageIndex);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	recordCommandBuffer(imageIndex);
	result = m_dixSwapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			m_Window.wasWindowResized()) {
		m_Window.resetWindowResizedFlag();
		recreateSwapChain();
		return;
	}
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}

} // namespace dix