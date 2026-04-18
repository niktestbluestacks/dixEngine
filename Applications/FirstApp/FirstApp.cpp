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
	createPipeline();
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
	glm::vec2 top) {
	if (depth <= 0) {
		vertices.push_back({ top });
		vertices.push_back({ right });
		vertices.push_back({ left });
	}
	else {
		auto leftTop = 0.5f * (left + top);
		auto rightTop = 0.5f * (right + top);
		auto leftRight = 0.5f * (left + right);
		sierpinski(vertices, depth - 1, left, leftRight, leftTop);
		sierpinski(vertices, depth - 1, leftRight, right, rightTop);
		sierpinski(vertices, depth - 1, leftTop, rightTop, top);
	}
}

void FirstApp::loadModels() {
	std::vector <Model::Vertex> vertices{};
	sierpinski(vertices, 7, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });
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
	auto pipelineConfig = 
		Pipeline::defaultPipelineConfigInfo(
			m_dixSwapChain.width(), 
			m_dixSwapChain.height());

	pipelineConfig.renderPass = m_dixSwapChain.getRenderPass();
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique <Pipeline>(
		m_dixDevice,
		toShaderPath("SimpleShader/simple_shader.vert.spv"),
		toShaderPath("SimpleShader/simple_shader.frag.spv"),
		pipelineConfig);
}

void FirstApp::createCommandBuffers() {

	m_commandBuffers.resize(m_dixSwapChain.imageCount());

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

	for (int i = 0, commandBuffersSize = static_cast <int> (m_commandBuffers.size()); i < commandBuffersSize; ++i) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(
				m_commandBuffers[i], 
				&beginInfo) !=
				VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_dixSwapChain.getRenderPass();
		renderPassInfo.framebuffer = m_dixSwapChain.getFrameBuffer(i);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_dixSwapChain.getSwapChainExtent();

		std::array <VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast <uint32_t> (clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_pipeline->bind(m_commandBuffers[i]);
		m_dixModel->bind(m_commandBuffers[i]);
		m_dixModel->draw(m_commandBuffers[i]);

		vkCmdEndRenderPass(m_commandBuffers[i]);
		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void FirstApp::drawFrame() {
	uint32_t imageIndex;
	auto result = m_dixSwapChain.acquireNextImage(&imageIndex);
	
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	result = m_dixSwapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}

} // namespace dix