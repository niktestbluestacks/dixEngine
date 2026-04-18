#include <FirstApp/FirstApp.hpp>
#include <Utils/Converter.hpp>

// std
#include <stdexcept>

namespace dix {
FirstApp::FirstApp() {
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

FirstApp::~FirstApp() {
	vkDestroyPipelineLayout(m_dixDevice.device(), m_pipelineLayout, nullptr);
}

void FirstApp::run(void) {
	while (!m_Window.shouldClose()) {
		glfwPollEvents();
	}
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

void FirstApp::createCommandBuffers() {}

void FirstApp::drawFrame() {}

} // namespace dix