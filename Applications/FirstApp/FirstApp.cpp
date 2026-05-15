// dix
#include <FirstApp/FirstApp.hpp>

#include <FirstApp/AppContext.hpp>
#include <DixCamera/DixCamera.hpp>
#include <Input/Keyboard/KeyboardAndMouseController.hpp>
#include <Utils/Converter.hpp>
#include <Logger/Logger.hpp>
#include <DixUI/DixFpsCounter.hpp>
#include <DixUI/DixTimeCounter.hpp>
#include <DixUI/DixPlayerInfo.hpp>
#include <Sound/DixAudio.hpp>
#include <Utils/DixRandom.hpp>

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
	m_context->~AppContext();
	m_gameObjects.clear();
	DixLogInfo("FirstApp closed successfully!");
}

void FirstApp::run(void) {
	DixCamera dixcamera{};
	dixcamera.setViewTarget(playerPosition, playerLookAt);

	auto viewerObject = GameObject::createGameObject();
	KeyboardAndMouseController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();

	auto sound = getRandomFile(toAudioPath(""));

	DixLogInfo("Background theme: {}", sound);

	m_sounds["Background theme"] = 
		DixAudio(sound);

	m_sounds["Background theme"].play(true);

	while (!m_context->shouldClose()) {
		m_context->pollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration <float, std::chrono::seconds::period> (newTime - currentTime).count();
		currentTime = newTime;

		frameTime = glm::min(frameTime, MAX_FRAME_TIME);

		cameraController.moveInPlaneXZ(m_context->getGLFWwindow(), frameTime, viewerObject);
		playerPosition = viewerObject.transform.translation;
		playerLookAt = viewerObject.transform.rotation;
		dixcamera.setViewYXZ(playerPosition, playerLookAt);

		float aspect = m_context->getAspectRatio();
		dixcamera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

		try {
			m_context->drawFrame(dixcamera, frameTime, m_gameObjects, playerPosition);
		}
		catch (const std::exception& e) {
			DixLogErr("Render error: {}", e.what());
			break;
		}
	}
}

void FirstApp::loadGameObjects() {
	std::random_device rd;
    
    std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> dist(0.0f, 10.0f);

	for (const auto& entry : std::filesystem::directory_iterator(MODEL_FILEPATH_RELATIVE)) {

		if (std::filesystem::is_directory(entry.path()) ||
			std::filesystem::absolute(entry.path()).string().back() == 'l') {
			continue;
		}
		DixLogDebug("Loading model: {}", std::filesystem::absolute(entry).string());
		std::shared_ptr <Model> dixModel = Model::createModelFromFile(
			m_context->device(), 
			std::filesystem::absolute(entry).string(),
			m_context->getDescriptorPool(),
			m_context->getModelSetLayout()
		);

		auto gameObj = GameObject::createGameObject();
		gameObj.model = dixModel;
		gameObj.transform.translation = { dist(gen), dist(gen), dist(gen) };
		gameObj.transform.scale = { 1.f, 1.f, 1.f };
		m_gameObjects["SimpleRenderSystem"].push_back(std::move(gameObj));
	}
}

void FirstApp::loadUIElements(void) {
	auto fps = std::make_unique<DixFpsCounter>(
		DixUIInfo {
		*m_context->getUIRenderer(),
		m_context->getExtent()
		// "",
		// "UI/font.txt",
		// "UI/font02.tga"
        }
    );
	m_context->addUIElement(std::move(fps));
    auto timeCounter = std::make_unique<DixTimeCounter>(
        DixUIInfo {
            *m_context->getUIRenderer(),
			m_context->getExtent(),
			// "",
			// "UI/font.txt",
			// "UI/font02.tga" 
		}
    );
    m_context->addUIElement(std::move(timeCounter));

	auto playerInfo = std::make_unique<DixPlayerInfo>(
		DixUIInfo {
			*m_context->getUIRenderer(),
			m_context->getExtent(),
			// "",
			// "UI/font.txt",
			// "UI/font02.tga" 
		},
		playerPosition
	);
	m_context->addUIElement(std::move(playerInfo));
}

} // namespace dix