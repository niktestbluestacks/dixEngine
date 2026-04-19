// dix
#include <FirstApp/FirstApp.hpp>
#include <Input/Keyboard/KeyboardController.hpp>
#include <Camera/Camera.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cstdint>
#include <chrono>
#include <stdexcept>

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
    Camera camera{};

    camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

    auto viewerObject = GameObject::createGameObject();
    KeyboardController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

	while (!m_Window.shouldClose()) {
		glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration <float, std::chrono::seconds::period> (newTime - currentTime).count();
        currentTime = newTime;

        frameTime = glm::min(frameTime, MAX_FRAME_TIME);

        cameraController.modeInPlaneXZ(m_Window.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = m_dixRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f);
        if (auto commandBuffer = m_dixRenderer.beginFrame()) {
			
			// begin offscreen shadow pass
			// render shadow casting objects
			// end offscreen shadow pass

			m_dixRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
			m_dixRenderer.endSwapChainRenderPass(commandBuffer);
			m_dixRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(m_dixDevice.device());
}

//void FirstApp::sierpinski(
//		std::vector<Model::Vertex>& vertices,
//		int depth,
//		glm::vec2 left,
//		glm::vec2 right,
//		glm::vec2 top,
//		glm::vec3 leftColor,
//		glm::vec3 rightColor,
//		glm::vec3 topColor) {
//	if (depth <= 0) {
//		vertices.push_back({ top, topColor });
//		vertices.push_back({ right, rightColor });
//		vertices.push_back({ left, leftColor });
//	}
//	else {
//		auto leftTop = 0.5f * (left + top);
//		auto rightTop = 0.5f * (right + top);
//		auto leftRight = 0.5f * (left + right);
//
//		auto leftTopColor = 0.5f * (leftColor + topColor);
//		auto rightTopColor = 0.5f * (rightColor + topColor);
//		auto leftRightColor = 0.5f * (leftColor + rightColor);
//
//		sierpinski(vertices, depth - 1, left, leftRight, leftTop, leftColor, leftRightColor, leftTopColor);
//		sierpinski(vertices, depth - 1, leftRight, right, rightTop, leftRightColor, rightColor, rightTopColor);
//		sierpinski(vertices, depth - 1, leftTop, rightTop, top, leftTopColor, rightTopColor, topColor);
//	}
//}

std::unique_ptr<Model> createCubeModel(EngineDevice& device, glm::vec3 offset) {
    Model::Builder modelBuilder{};
    modelBuilder.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto& v : modelBuilder.vertices) {
        v.position += offset;
    }

    modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

    return std::make_unique<Model>(device, modelBuilder);
}

void FirstApp::loadGameObjects() {
    std::shared_ptr <Model> dixModel = createCubeModel(m_dixDevice, {.0f, .0f, .0f});

    auto cube = GameObject::createGameObject();
    cube.model = dixModel;
    cube.transform.translation = { .0f, .0f, 2.5f };
    cube.transform.scale = { .5f, .5f, .5f };
    m_gameObjects.push_back(std::move(cube));
}

} // namespace dix