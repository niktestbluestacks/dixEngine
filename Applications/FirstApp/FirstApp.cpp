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

void FirstApp::loadGameObjects() {
	std::shared_ptr <Model> dixModel = Model::createModelFromFile(m_dixDevice, toModelPath("smooth_vase.obj"));

    auto gameObj = GameObject::createGameObject();
    gameObj.model = dixModel;
    gameObj.transform.translation = { .0f, .0f, 2.5f };
	gameObj.transform.scale = glm::vec3{ 3.f };
    m_gameObjects.push_back(std::move(gameObj));
}

} // namespace dix