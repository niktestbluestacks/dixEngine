// dix
#include <FirstApp/FirstApp.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <cstdint>
#include <array>

namespace dix {

FirstApp::FirstApp() {
	Logger::get().log(Logger::INFO, "Initiating the application...");
	Logger::get().log(Logger::INFO, "-----------------------------");
	loadGameObjects();
	Logger::get().log(Logger::INFO, "Application initialized succsesfully!");
}

FirstApp::~FirstApp() {
	Logger::get().log(Logger::INFO, "Closing the application...");
	Logger::get().log(Logger::INFO, "--------------------------");
	Logger::get().log(Logger::INFO, "Application has been closed succsesfully!");
}

void FirstApp::run(void) {
	SimpleRenderSystem simpleRenderSystem{ m_dixDevice, m_dixRenderer.getSwapChainRenderPass() };
	while (!m_Window.shouldClose()) {
		glfwPollEvents();
		
		if (auto commandBuffer = m_dixRenderer.beginFrame()) {
			
			// begin offscreen shadow pass
			// render shadow casting objects
			// end offscreen shadow pass

			m_dixRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects);
			m_dixRenderer.endSwapChainRenderPass(commandBuffer);
			m_dixRenderer.endFrame();
		}
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

void FirstApp::loadGameObjects() {
	std::vector <Model::Vertex> vertices = {
		{{ 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
		{{ 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }},
		{{ -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }}
	};

	//sierpinski(vertices, 8, { 0.0f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

	auto dixModel = std::make_shared <Model>(m_dixDevice, vertices);

	auto triangle = GameObject::createGameObject();
	triangle.model = dixModel;
	triangle.color = { .1f, .8f, .1f };
	triangle.transform2d.translation.x = .2f;
	triangle.transform2d.scale = { 2.f, .5f };
	triangle.transform2d.rotation = .25f * glm::two_pi <float>();

	m_gameObjects.push_back(std::move(triangle));
}

} // namespace dix