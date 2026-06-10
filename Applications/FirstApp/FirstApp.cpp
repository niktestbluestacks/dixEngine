// dix
#include <FirstApp/AppContext.hpp>
#include <FirstApp/FirstApp.hpp>

namespace dix {

FirstApp::FirstApp(void) {
    DixLogInfo("Initializing FirstApp...");
    DixLogInfo("Loading Game Objects");
    // FirstApp focuses on game objects and game logic only.
    loadConsoleCommands();
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
    KeyboardAndMouseController cameraController{m_context->getDixWindow()};

    auto currentTime = std::chrono::high_resolution_clock::now();

    auto sound = getRandomFile(toAudioPath(""));

    DixLogInfo("Background theme: {}", sound);

    m_sounds["Background theme"] = DixAudio(sound);

    m_sounds["Background theme"].play(true);

    bool initialStructureRecorded = false;

    auto& console = DixConsole::getDixConsole();
    while (!m_context->shouldClose()) {
        console.newFrame();
        m_context->pollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(
                newTime - currentTime)
                .count();
        currentTime = newTime;

        frameTime = glm::min(frameTime, MAX_FRAME_TIME);
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
            cameraController.moveInPlaneXZ(frameTime, viewerObject);
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

    auto consoleUI = std::make_unique<DixConsoleUI>(DixUIInfo{
        *m_context->getUIRenderer(),
        m_context->getExtent(),
    });

    auto* consoleUIPtr = consoleUI.get();
    m_context->addUIElement(std::move(consoleUI));

    auto& console = CurrentConsole::getDixConsole();
    console.setConsoleUI(consoleUIPtr);
    console.setPosition(0.f, 600.f, 600.0f, 600.f);

    consoleUIPtr->setHistoryRef(&console.getHistory());
    consoleUIPtr->setInputBufferCallback(
        [&console]() { return console.getInputBuffer(); });

    m_context->getDixWindow().bindKey(
        GLFW_KEY_GRAVE_ACCENT, [&console]() { console.toggleConsole(); }, true);

    m_context->getDixWindow().bindKey(
        GLFW_KEY_BACKSPACE,
        [&console]() {
            if (console.isVisible()) {
                console.backspace();
            }
        },
        false, nullptr, [&console]() { console.backspaceRealeased(); });

    m_context->getDixWindow().bindKey(GLFW_KEY_ENTER, [&console]() {
        if (console.isVisible()) {
            console.enterCommand();
        }
    });

    m_context->getDixWindow().setCharCallback([&console](char c) {
        if (console.isVisible() && c != '`') {
            console.addCharacter(c);
        }
    });

    console.logInfo("Console initialized");

    m_context->getDixWindow().bindKey(GLFW_KEY_R, [this]() {
        static float keyCooldown = 0.f;
        static auto lastFrame = std::chrono::steady_clock::now();
        auto currentFrame = std::chrono::steady_clock::now();
        float frameTime =
            (currentFrame - lastFrame) / std::chrono_literals::operator""s(1);
        lastFrame = currentFrame;

        keyCooldown -= frameTime;

        if (keyCooldown > 0.f) {
            return;
        }

        if (!this->m_recording && !this->m_playing) {
            this->m_recording = true;
            getFrameRecorder().startRecording("recording.txt");
            DixLogInfo("Recording started - press R again to stop");
            keyCooldown = 0.3f;  // 300ms cooldown
        } else if (this->m_recording) {
            this->m_recording = false;
            getFrameRecorder().stopRecording();
            DixLogInfo("Recording stopped");
            keyCooldown = 0.3f;
        }
    });

    m_context->getDixWindow().bindKey(GLFW_KEY_P, [this]() {
        static float keyCooldown = 0.f;
        static auto lastFrame = std::chrono::steady_clock::now();
        auto currentFrame = std::chrono::steady_clock::now();
        float frameTime =
            (currentFrame - lastFrame) / std::chrono_literals::operator""s(1);
        lastFrame = currentFrame;

        keyCooldown -= frameTime;

        if (keyCooldown > 0.f) {
            return;
        }

        if (!this->m_playing && !this->m_recording) {
            this->m_playing = true;
            this->m_initialGameObects = this->m_gameObjects;
            getFrameRecorder().startPlayback(
                "recording.txt", this->m_gameObjects, this->playerPosition,
                this->playerLookAt);
            DixLogInfo("Playback started - press P again to stop");
            keyCooldown = 0.3f;
        } else if (this->m_playing) {
            this->m_playing = false;
            getFrameRecorder().stopPlayback();
            this->m_gameObjects = this->m_initialGameObects;
            DixLogInfo("Playback stopped");
            keyCooldown = 0.3f;
        }
    });
}

void FirstApp::loadConsoleCommands(void) {
    auto& console = CurrentConsole::getDixConsole();
    console.register_function(
        "play", std::function{[&]() {
            static float keyCooldown = 0.f;
            static auto lastFrame = std::chrono::steady_clock::now();
            auto currentFrame = std::chrono::steady_clock::now();
            float frameTime = (currentFrame - lastFrame) /
                              std::chrono_literals::operator""s(1);
            lastFrame = currentFrame;

            keyCooldown -= frameTime;

            if (keyCooldown > 0.f) {
                return;
            }

            if (!this->m_playing && !this->m_recording) {
                this->m_playing = true;
                this->m_initialGameObects = this->m_gameObjects;
                getFrameRecorder().startPlayback(
                    "recording.txt", this->m_gameObjects, this->playerPosition,
                    this->playerLookAt);
                console.log("Playback started - use play again to stop");
                keyCooldown = 0.3f;
            } else if (this->m_playing) {
                this->m_playing = false;
                getFrameRecorder().stopPlayback();
                this->m_gameObjects = this->m_initialGameObects;
                console.log("Playback stopped");
                keyCooldown = 0.3f;
            }
        }});

    console.register_function(
        "record", std::function{[&]() {
            static float keyCooldown = 0.f;
            static auto lastFrame = std::chrono::steady_clock::now();
            auto currentFrame = std::chrono::steady_clock::now();
            float frameTime = (currentFrame - lastFrame) /
                              std::chrono_literals::operator""s(1);
            lastFrame = currentFrame;

            keyCooldown -= frameTime;

            if (keyCooldown > 0.f) {
                return;
            }

            if (!this->m_recording && !this->m_playing) {
                this->m_recording = true;
                getFrameRecorder().startRecording("recording.txt");
                console.log("Recording started - use record to stop recording");
                keyCooldown = 0.3f;  // 300ms cooldown
            } else if (this->m_recording) {
                this->m_recording = false;
                getFrameRecorder().stopRecording();
                console.log("Recording stopped");
                keyCooldown = 0.3f;
            }
        }});

    console.register_function(
        "sound", std::function([&](const std::vector<std::string>& arguments) {
            auto volume = m_sounds["Background theme"].getVolume();
            bool play = m_sounds["Background theme"].isPlaying();
            bool isNextVolume = false;
            for (auto& elem : arguments) {
                auto res = string_to_num(elem);
                std::visit(
                    [&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<std::string, T>) {
                            isNextVolume = arg == "volume";
                            if (arg == "resume") {
                                m_sounds["Background theme"].resume();
                            } else if (arg == "pause") {
                                m_sounds["Background theme"].pause();
                            }
                        } else if constexpr (std::is_same_v<int, T>) {
                            if (isNextVolume) {
                                isNextVolume = false;
                                m_sounds["Background theme"].setVolume(
                                    static_cast<float>(arg) / 100.f);
                            }
                        } else if constexpr (std::is_same_v<float, T>) {
                            if (isNextVolume) {
                                isNextVolume = false;
                                m_sounds["Background theme"].setVolume(arg);
                            }
                        }
                    },
                    res);
            }
        }));

    console.register_function(
        "particle_emitter",
        std::function{[&](const std::vector<std::string>& arguments) {
            std::string flag{"--bouncy"};

            bool is_bouncy = false;
            glm::vec3 position{0.f};
            std::array<bool, 4> cleared{};
            int amount = 0;
            for (auto& elem : arguments) {
                auto result = string_to_num(elem);
                std::visit(
                    [&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int>) {
                            if (!cleared[3]) {
                                cleared[3] = true;
                                amount = arg;
                            }
                        } else if constexpr (std::is_same_v<T, float>) {
                            if (!cleared[0]) {
                                cleared[0] = true;
                                position.x = arg;
                            } else if (!cleared[1]) {
                                cleared[1] = true;
                                position.y = arg;
                            } else if (!cleared[2]) {
                                cleared[2] = true;
                                position.z = arg;
                            }
                        } else if constexpr (std::is_same_v<T, std::string>) {
                            if (arg == "--bouncy") {
                                is_bouncy = true;
                            }
                        }
                    },
                    result);
            }
            if (is_bouncy) {
                m_gameObjects["BouncyParticleRenderSystem"].push_back(std::move(
                    m_context->getRenderSystem<BouncyParticleRenderSystem>()
                        .createParticleEmitter(position, amount)));
            } else {
                m_gameObjects["ParticleRenderSystem"].push_back(
                    std::move(m_context->getRenderSystem<ParticleRenderSystem>()
                                  .createParticleEmitter(position, amount)));
            }
        }});
}

}  // namespace dix