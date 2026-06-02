// dix
#include <FirstApp/AppContext.hpp>
#include <FirstApp/FirstApp.hpp>

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
    m_context.reset();
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

    m_sounds["Background theme"] = DixAudio(sound);

    m_sounds["Background theme"].play(true);

    bool initialStructureRecorded = false;

    while (!m_context->shouldClose()) {
        m_context->pollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(
                newTime - currentTime)
                .count();
        currentTime = newTime;

        frameTime = glm::min(frameTime, MAX_FRAME_TIME);

        // Handle recorder input (R to record, P to playback)
        handleRecorderInput(frameTime);

        // If playing back, use recorded camera data and object states
        if (m_playing) {
            auto& recorder = getFrameRecorder();
            if (!recorder.updatePlayback(m_gameObjects, playerPosition,
                                         playerLookAt)) {
                DixLogInfo("Playback finished");
                m_playing = false;
                recorder.stopPlayback();
            } else {
                if (recorder.getCurrentFrame()) {
                    frameTime = recorder.getCurrentFrame()->frameTime;
                }
                dixcamera.setViewYXZ(playerPosition, playerLookAt);
            }
        } else {
            cameraController.moveInPlaneXZ(m_context->getGLFWwindow(),
                                           frameTime, viewerObject);
            playerPosition = viewerObject.transform.translation;
            playerLookAt = viewerObject.transform.rotation;
            dixcamera.setViewYXZ(playerPosition, playerLookAt);
        }

        float aspect = m_context->getAspectRatio();
        dixcamera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f,
                                           100.f);

        // Record frame if recording
        if (m_recording) {
            auto& recorder = getFrameRecorder();
            // Record initial structure once at the start of recording
            if (!initialStructureRecorded) {
                recorder.recordInitialStructure(m_gameObjects);
                initialStructureRecorded = true;
            }
            recorder.recordFrame(m_gameObjects, playerPosition, playerLookAt,
                                 frameTime);
        }

        try {
            m_context->drawFrame(dixcamera, frameTime, m_gameObjects,
                                 playerPosition);
        } catch (const std::exception& e) {
            DixLogErr("Render error: {}", e.what());
            break;
        }
    }

    // Stop recording if app closes while recording
    if (m_recording) {
        getFrameRecorder().stopRecording();
        m_recording = false;
    }
}

void FirstApp::loadGameObjects() {
    DixLogDebug("Maximum allocation size for physical device: {}",
                m_context->device().getMaximumAllocationSize());
    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> dist(0.0f, 10.0f);

    for (const auto& entry :
         std::filesystem::directory_iterator(MODEL_FILEPATH_RELATIVE)) {
        if (std::filesystem::is_directory(entry.path()) ||
            std::filesystem::absolute(entry.path()).string().back() == 'l') {
            continue;
        }
        DixLogDebug("Loading model: {}",
                    std::filesystem::absolute(entry).string());
        std::shared_ptr<Model> dixModel = Model::createModelFromFile(
            m_context->device(), std::filesystem::absolute(entry).string(),
            m_context->getDescriptorPool(), m_context->getModelSetLayout());

        auto gameObj = GameObject::createGameObject();
        gameObj.model = dixModel;
        gameObj.transform.translation = {dist(gen), dist(gen), dist(gen)};
        gameObj.transform.scale = {1.f, 1.f, 1.f};
        m_gameObjects["SimpleRenderSystem"].push_back(std::move(gameObj));
    }

    // particle emitter
    auto particleEmitter = GameObject::createGameObject();
    particleEmitter.transform.translation = glm::vec3{0.f, 0.f, 0.f};
    m_gameObjects["ParticleRenderSystem"].push_back(std::move(particleEmitter));
    m_context->getRenderSystem<ParticleRenderSystem>().createParticleEmitter(
        glm::vec3{0.f, 50.f, 0.f}, 500);

    // bouncy particle emitter
    auto bouncyParticleEmitter = GameObject::createGameObject();
    particleEmitter.transform.translation = glm::vec3{0.f, 0.f, 0.f};
    m_gameObjects["BouncyParticleRenderSystem"].push_back(
        std::move(particleEmitter));
    m_context->getRenderSystem<BouncyParticleRenderSystem>()
        .createParticleEmitter(glm::vec3{0.f, 0.f, 0.f}, 500);

    // skybox
    const auto entry = toModelPath("skybox.obj");
    DixLogInfo("Skybox model is: {}", entry);

    std::shared_ptr<Model> dixModel = Model::createModelFromFile(
        m_context->device(), std::filesystem::absolute(entry).string(),
        m_context->getDescriptorPool(), m_context->getModelSetLayout());

    auto Skybox = GameObject::createGameObject();
    Skybox.model = dixModel;
    dixModel.reset();
    m_gameObjects["SkyboxRenderSystem"].push_back(std::move(Skybox));
}

void FirstApp::loadUIElements(void) {
    m_context->getDixWindow().setWindowIcon(toModelPath("Images/icon.ico"));

    auto fps = std::make_unique<DixFpsCounter>(DixUIInfo{
        *m_context->getUIRenderer(), m_context->getExtent()
        // "",
        // "UI/font.txt",
        // "UI/font02.tga"
    });
    m_context->addUIElement(std::move(fps));
    auto timeCounter = std::make_unique<DixTimeCounter>(DixUIInfo{
        *m_context->getUIRenderer(),
        m_context->getExtent(),
        // "",
        // "UI/font.txt",
        // "UI/font02.tga"
    });
    m_context->addUIElement(std::move(timeCounter));

    auto playerInfo = std::make_unique<DixPlayerInfo>(
        DixUIInfo{
            *m_context->getUIRenderer(),
            m_context->getExtent(),
            // "",
            // "UI/font.txt",
            // "UI/font02.tga"
        },
        playerPosition);
    m_context->addUIElement(std::move(playerInfo));
}

void FirstApp::handleRecorderInput(float frameTime) {
    static float keyCooldown = 0.f;
    keyCooldown -= frameTime;

    if (keyCooldown > 0.f) {
        return;
    }

    GLFWwindow* window = m_context->getGLFWwindow();

    // Press R to start/stop recording
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!m_recording && !m_playing) {
            m_recording = true;
            getFrameRecorder().startRecording("recording.txt");
            DixLogInfo("Recording started - press R again to stop");
            keyCooldown = 0.3f;  // 300ms cooldown
        } else if (m_recording) {
            m_recording = false;
            getFrameRecorder().stopRecording();
            DixLogInfo("Recording stopped");
            keyCooldown = 0.3f;
        }
    }

    // Press P to start/stop playback
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!m_playing && !m_recording) {
            m_playing = true;
            getFrameRecorder().startPlayback("recording.txt", m_gameObjects,
                                             playerPosition, playerLookAt);
            DixLogInfo("Playback started - press P again to stop");
            keyCooldown = 0.3f;
        } else if (m_playing) {
            m_playing = false;
            getFrameRecorder().stopPlayback();
            DixLogInfo("Playback stopped");
            keyCooldown = 0.3f;
        }
    }
}

}  // namespace dix