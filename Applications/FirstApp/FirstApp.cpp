// dix
#include <FirstApp/FirstApp.hpp>

#include <FirstApp/AppContext.hpp>
#include <DixCamera/DixCamera.hpp>
#include <Input/Keyboard/KeyboardController.hpp>
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
#include <iostream>
#include <stdexcept>

namespace dix {

FirstApp::FirstApp() {
	// FirstApp focuses on game objects and game logic only.
	loadGameObjects();
}

FirstApp::~FirstApp() = default;

void FirstApp::run(void) {
	DixCamera dixcamera{};
	dixcamera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

	auto viewerObject = GameObject::createGameObject();
	KeyboardController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();

	while (!m_context.shouldClose()) {
		m_context.pollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration <float, std::chrono::seconds::period> (newTime - currentTime).count();
		currentTime = newTime;

		frameTime = glm::min(frameTime, MAX_FRAME_TIME);

		cameraController.modeInPlaneXZ(m_context.getGLFWwindow(), frameTime, viewerObject);
		dixcamera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = m_context.getAspectRatio();
		dixcamera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

        // Delegate rendering details to AppContext to keep FirstApp focused on logic
		try {
			m_context.drawFrame(dixcamera, frameTime, m_gameObjects);
		}
		catch (const std::exception& e) {
			std::cerr << "Render error: " << e.what() << std::endl;
			break; // exit run loop on fatal render errors
		}
	}
}

void FirstApp::loadGameObjects() {
    // Use the device from AppContext for loading models/resources
	std::shared_ptr <Model> dixModel = Model::createModelFromFile(m_context.device(), toModelPath("colored_cube.obj"));

    auto gameObj = GameObject::createGameObject();
    gameObj.model = dixModel;
    gameObj.transform.translation = { .0f, .5f, 2.5f };
	gameObj.transform.scale = { 3.f, 3.f, 3.f };
    m_gameObjects.push_back(std::move(gameObj));
}

} // namespace dix