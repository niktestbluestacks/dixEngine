// dix
#include <FirstApp/FirstApp.hpp>

#include <FirstApp/AppContext.hpp>
#include <DixCamera/DixCamera.hpp>
#include <Input/Keyboard/KeyboardController.hpp>
#include <Utils/Converter.hpp>
#include <Logger/Logger.hpp>
#include <DixUI/DixFpsCounter.hpp>
#include <DixUI/DixTimeCounter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <chrono>
#include <string>
#include <filesystem>
#include <random>

namespace dix {

FirstApp::FirstApp(void) {
	DixLogInfo("Initializing FirstApp...");
	DixLogInfo("Loading Game Objects");
	// FirstApp focuses on game objects and game logic only.
	loadGameObjects();
	loadUIElements();
	DixLogInfo("FirstApp initialized successfully!");
}

FirstApp::~FirstApp(void) {
	DixLogInfo("Closing FirstApp...");
}

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
			DixLogErr("Render error: " + static_cast <std::string> (e.what()));
			break; // exit run loop on fatal render errors
		}
	}
}

void FirstApp::loadGameObjects() {
    // Use the device from AppContext for loading models/resources
	std::random_device rd;
    
    // 2. Initialize the Mersenne Twister engine
    std::mt19937 gen(rd());
    
    // 3. Define the distribution range [min, max)
    // For floats, use std::uniform_real_distribution<float>
    std::uniform_real_distribution<float> dist(0.0f, 10.0f);



	for (const auto& entry : std::filesystem::directory_iterator(MODEL_FILEPATH_RELATIVE)) {

		if (std::filesystem::is_directory(entry.path()) ||
		  std::filesystem::absolute(entry.path()).string().back() == 'l') {
			continue;
		}
		DixLogDebug("Loading model: " + std::filesystem::absolute(entry).string());
		std::shared_ptr <Model> dixModel = Model::createModelFromFile(
				m_context.device(), 
				std::filesystem::absolute(entry).string()
		);

		auto gameObj = GameObject::createGameObject();
		gameObj.model = dixModel;
		gameObj.transform.translation = { dist(gen), dist(gen), dist(gen) };
		gameObj.transform.scale = { 1.f, 1.f, 1.f };
		m_gameObjects.push_back(std::move(gameObj));
	}
}

void FirstApp::loadUIElements(void) {
	auto fps = std::make_unique<DixFpsCounter>(
		DixUIInfo {
		*m_context.getUIRenderer(),
		m_context.getExtent()
		// "",
		// "UI/font.txt",
		// "UI/font02.tga"
        }
    );
	m_context.addUIElement(std::move(fps));
    auto timeCounter = std::make_unique<DixTimeCounter>(
        DixUIInfo {
            *m_context.getUIRenderer(),
			m_context.getExtent(),
			// "",
			// "UI/font.txt",
			// "UI/font02.tga" 
		}
    );
    m_context.addUIElement(std::move(timeCounter));
}

} // namespace dix