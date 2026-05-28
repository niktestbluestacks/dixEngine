// dix
#include <FrameRecorder/FrameRecorder.hpp>
#include <Logger/Logger.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Model/Model.hpp>

// std
#include <cstring>
#include <iomanip>
#include <sstream>

namespace dix {

// Global singleton instance
static FrameRecorder g_frameRecorder;

FrameRecorder& getFrameRecorder() { return g_frameRecorder; }

std::string FrameRecorder::getModelPath(const GameObject& obj) const {
    // Since we can't easily get the model path from a shared_ptr<Model>,
    // we'll use a placeholder. In a real implementation, Model would expose
    // this.
    if (obj.model) {
        return "model_" +
               std::to_string(reinterpret_cast<uintptr_t>(obj.model.get()));
    }
    return "no_model";
}

void FrameRecorder::startRecording(const std::string& filename) {
    m_filename = filename;
    m_outputStream.open(filename, std::ios::trunc);
    if (!m_outputStream.is_open()) {
        DixLogErr("Failed to open recording file: {}", filename);
        m_isRecording = false;
        return;
    }
    m_isRecording = true;
    m_isPlaying = false;
    m_frames.clear();
    m_prevTranslations.clear();
    m_prevRotations.clear();
    m_prevScales.clear();
    m_initialStructure.gameObjectsByRenderSystem.clear();
}

void FrameRecorder::stopRecording() {
    if (!m_isRecording) {
        return;
    }

    // Write initial structure first (if not already written)
    writeInitialStructure();

    // Write all accumulated frames
    size_t frameNumber = 0;
    for (const auto& frame : m_frames) {
        writeFrame(frame, frameNumber++);
    }

    m_outputStream.close();
    m_isRecording = false;

    // Clear recording state but don't crash - allow program to continue
    m_frames.clear();
    m_prevTranslations.clear();
    m_prevRotations.clear();
    m_prevScales.clear();
    m_initialStructure.gameObjectsByRenderSystem.clear();
}

void FrameRecorder::recordInitialStructure(
    const std::unordered_map<std::string, std::vector<GameObject>>&
        gameObjects) {
    if (!m_isRecording) {
        return;
    }

    m_initialStructure.gameObjectsByRenderSystem.clear();

    for (const auto& [renderSystemName, objects] : gameObjects) {
        auto& recordedObjects =
            m_initialStructure.gameObjectsByRenderSystem[renderSystemName];
        recordedObjects.reserve(objects.size());

        for (const auto& obj : objects) {
            RecordedGameObject recorded;
            recorded.id = obj.getId();
            recorded.translation = obj.transform.translation;
            recorded.rotation = obj.transform.rotation;
            recorded.scale = obj.transform.scale;
            recorded.modelPath = getModelPath(obj);
            recordedObjects.push_back(recorded);

            // Store initial state for change detection
            m_prevTranslations[recorded.id] = recorded.translation;
            m_prevRotations[recorded.id] = recorded.rotation;
            m_prevScales[recorded.id] = recorded.scale;
        }
    }
}

void FrameRecorder::recordFrame(
    const std::unordered_map<std::string, std::vector<GameObject>>& gameObjects,
    const glm::vec3& cameraPosition, const glm::vec3& cameraLookAt,
    float frameTime) {
    if (!m_isRecording) {
        return;
    }

    RecordedFrame frame;
    frame.frameTime = frameTime;
    frame.cameraPosition = cameraPosition;
    frame.cameraLookAt = cameraLookAt;

    // Find changed objects by comparing with previous state
    for (const auto& [renderSystemName, objects] : gameObjects) {
        for (const auto& obj : objects) {
            auto id = obj.getId();
            bool changed = false;

            // Check translation change
            auto prevTransIt = m_prevTranslations.find(id);
            if (prevTransIt == m_prevTranslations.end() ||
                prevTransIt->second != obj.transform.translation) {
                frame.translations[id] = obj.transform.translation;
                changed = true;
            }

            // Check rotation change
            auto prevRotIt = m_prevRotations.find(id);
            if (prevRotIt == m_prevRotations.end() ||
                prevRotIt->second != obj.transform.rotation) {
                frame.rotations[id] = obj.transform.rotation;
                changed = true;
            }

            // Check scale change
            auto prevScaleIt = m_prevScales.find(id);
            if (prevScaleIt == m_prevScales.end() ||
                prevScaleIt->second != obj.transform.scale) {
                frame.scales[id] = obj.transform.scale;
                changed = true;
            }

            if (changed) {
                frame.changedObjectIds.push_back(id);
            }

            // Update previous state
            m_prevTranslations[id] = obj.transform.translation;
            m_prevRotations[id] = obj.transform.rotation;
            m_prevScales[id] = obj.transform.scale;
        }
    }

    m_frames.push_back(std::move(frame));
}

void FrameRecorder::writeInitialStructure() {
    if (!m_outputStream.is_open()) {
        return;
    }

    // Write magic header
    m_outputStream << "FRAME_RECORDER_V1" << std::endl;

    // Write number of render systems
    m_outputStream << "RenderSystems: "
                   << m_initialStructure.gameObjectsByRenderSystem.size()
                   << std::endl;

    // Write each render system's objects
    for (const auto& [renderSystemName, objects] :
         m_initialStructure.gameObjectsByRenderSystem) {
        m_outputStream << "  RenderSystem: " << renderSystemName << std::endl;
        m_outputStream << "    ObjectCount: " << objects.size() << std::endl;

        // Write each object
        for (size_t i = 0; i < objects.size(); ++i) {
            const auto& obj = objects[i];
            m_outputStream << "    Object[" << i << "]: ID=" << obj.id
                           << " Model=" << obj.modelPath << std::endl;
            m_outputStream << "      Translation: " << obj.translation.x << " "
                           << obj.translation.y << " " << obj.translation.z
                           << std::endl;
            m_outputStream << "      Rotation: " << obj.rotation.x << " "
                           << obj.rotation.y << " " << obj.rotation.z
                           << std::endl;
            m_outputStream << "      Scale: " << obj.scale.x << " "
                           << obj.scale.y << " " << obj.scale.z << std::endl;
        }
    }

    // Write separator before frames
    m_outputStream << "FRAMES_START" << std::endl;
    m_outputStream << "TotalFrames: " << m_frames.size() << std::endl;
}

void FrameRecorder::writeFrame(const RecordedFrame& frame, size_t frameNumber) {
    if (!m_outputStream.is_open()) {
        return;
    }

    // Write frame header: "Frame: {N}"
    m_outputStream << "Frame: " << frameNumber << std::endl;

    // Write frame time
    m_outputStream << "  FrameTime: " << frame.frameTime << std::endl;

    // Write camera data
    m_outputStream << "  CameraPosition: " << frame.cameraPosition.x << " "
                   << frame.cameraPosition.y << " " << frame.cameraPosition.z
                   << std::endl;
    m_outputStream << "  CameraLookAt: " << frame.cameraLookAt.x << " "
                   << frame.cameraLookAt.y << " " << frame.cameraLookAt.z
                   << std::endl;

    // Write number of changed objects
    m_outputStream << "  ChangedObjects: " << frame.changedObjectIds.size()
                   << std::endl;

    // Write each changed object's data
    for (auto id : frame.changedObjectIds) {
        m_outputStream << "    ObjectID: " << id << std::endl;

        if (frame.translations.count(id)) {
            const auto& t = frame.translations.at(id);
            m_outputStream << "      Translation: " << t.x << " " << t.y << " "
                           << t.z << std::endl;
        }
        if (frame.rotations.count(id)) {
            const auto& r = frame.rotations.at(id);
            m_outputStream << "      Rotation: " << r.x << " " << r.y << " "
                           << r.z << std::endl;
        }
        if (frame.scales.count(id)) {
            const auto& s = frame.scales.at(id);
            m_outputStream << "      Scale: " << s.x << " " << s.y << " " << s.z
                           << std::endl;
        }
    }
}

void FrameRecorder::startPlayback(
    const std::string& filename,
    std::unordered_map<std::string, std::vector<GameObject>>& gameObjects,
    glm::vec3& cameraPosition, glm::vec3& cameraLookAt) {
    m_filename = filename;
    m_inputStream.open(filename);
    if (!m_inputStream.is_open()) {
        DixLogErr("Failed to open recording file: {}", filename);
        // Don't throw - just return and let program continue
        m_isPlaying = false;
        m_isRecording = false;
        return;
    }

    m_isPlaying = true;
    m_isRecording = false;
    m_currentFrameIndex = 0;
    m_frames.clear();

    // Clear previous mappings
    m_playbackIdMap.clear();
    m_idToLocationMap.clear();

    // Read initial structure and recreate game objects
    if (!readInitialStructure()) {
        DixLogErr("Failed to read initial structure from recording");
        m_isPlaying = false;
        m_inputStream.close();
        return;
    }

    // Apply initial structure to gameObjects - preserve order exactly as
    // recorded BUG FIX A: Store original models before clearing, then reassign
    // them
    std::unordered_map<std::string, std::vector<std::shared_ptr<Model>>>
        originalModels;
    for (const auto& [renderSystemName, objects] : gameObjects) {
        auto& models = originalModels[renderSystemName];
        models.reserve(objects.size());
        for (const auto& obj : objects) {
            models.push_back(obj.model);  // Save the model shared_ptr
        }
    }

    gameObjects.clear();
    for (const auto& [renderSystemName, recordedObjects] :
         m_initialStructure.gameObjectsByRenderSystem) {
        auto& objects = gameObjects[renderSystemName];
        objects.clear();
        objects.reserve(recordedObjects.size());

        size_t objectIndex = 0;
        for (const auto& recorded : recordedObjects) {
            GameObject obj = GameObject::createGameObject();
            obj.transform.translation = recorded.translation;
            obj.transform.rotation = recorded.rotation;
            obj.transform.scale = recorded.scale;

            // Try to restore the original model if available
            auto modelsIt = originalModels.find(renderSystemName);
            if (modelsIt != originalModels.end() &&
                objectIndex < modelsIt->second.size()) {
                obj.model = modelsIt->second[objectIndex];
            }

            // Store the recorded ID mapping: index -> recorded ID
            m_playbackIdMap[{renderSystemName, objectIndex}] = recorded.id;
            objects.push_back(std::move(obj));
            objectIndex++;
        }
    }

    // Set initial camera position from first frame
    if (!m_frames.empty()) {
        cameraPosition = m_frames[0].cameraPosition;
        cameraLookAt = m_frames[0].cameraLookAt;
    }
}

void FrameRecorder::stopPlayback() {
    if (!m_isPlaying) {
        return;  // Already stopped, don't crash
    }

    if (m_inputStream.is_open()) {
        m_inputStream.close();
    }
    m_isPlaying = false;
    m_currentFrameIndex = 0;
    // Don't clear frames here - they may still be needed by caller
    // m_frames.clear();

    // Clear the ID mapping so next playback starts fresh
    m_playbackIdMap.clear();
    m_idToLocationMap.clear();
}

bool FrameRecorder::updatePlayback(
    std::unordered_map<std::string, std::vector<GameObject>>& gameObjects,
    glm::vec3& cameraPosition, glm::vec3& cameraLookAt) {
    if (!m_isPlaying || m_currentFrameIndex >= m_frames.size()) {
        return false;
    }

    const RecordedFrame& frame = m_frames[m_currentFrameIndex];

    // Update camera
    cameraPosition = frame.cameraPosition;
    cameraLookAt = frame.cameraLookAt;

    // Apply changes to game objects using the ID-to-location mapping
    for (const auto& [id, translation] : frame.translations) {
        auto it = m_idToLocationMap.find(id);
        if (it != m_idToLocationMap.end()) {
            const auto& [renderSystemName, objectIndex] = it->second;
            auto rsIt = gameObjects.find(renderSystemName);
            if (rsIt != gameObjects.end() &&
                objectIndex < rsIt->second.size()) {
                rsIt->second[objectIndex].transform.translation = translation;
            }
        }
    }

    for (const auto& [id, rotation] : frame.rotations) {
        auto it = m_idToLocationMap.find(id);
        if (it != m_idToLocationMap.end()) {
            const auto& [renderSystemName, objectIndex] = it->second;
            auto rsIt = gameObjects.find(renderSystemName);
            if (rsIt != gameObjects.end() &&
                objectIndex < rsIt->second.size()) {
                rsIt->second[objectIndex].transform.rotation = rotation;
            }
        }
    }

    for (const auto& [id, scale] : frame.scales) {
        auto it = m_idToLocationMap.find(id);
        if (it != m_idToLocationMap.end()) {
            const auto& [renderSystemName, objectIndex] = it->second;
            auto rsIt = gameObjects.find(renderSystemName);
            if (rsIt != gameObjects.end() &&
                objectIndex < rsIt->second.size()) {
                rsIt->second[objectIndex].transform.scale = scale;
            }
        }
    }

    m_currentFrameIndex++;
    return true;
}

const RecordedFrame* FrameRecorder::getCurrentFrame() const {
    if (m_currentFrameIndex < m_frames.size()) {
        return &m_frames[m_currentFrameIndex];
    }
    return nullptr;
}

bool FrameRecorder::readInitialStructure() {
    if (!m_inputStream.is_open()) {
        DixLogErr("Input stream not open");
        return false;
    }

    // Read and verify header line
    std::string headerLine;
    if (!std::getline(m_inputStream, headerLine)) {
        DixLogErr("Failed to read header line");
        return false;
    }
    // Trim whitespace (header has no leading spaces)
    headerLine.erase(headerLine.find_last_not_of(" \t\r\n") + 1);
    if (headerLine != "FRAME_RECORDER_V1") {
        DixLogErr("Invalid header: '{}'", headerLine);
        return false;
    }

    // Read number of render systems
    std::string line;
    if (!std::getline(m_inputStream, line)) {
        DixLogErr("Failed to read RenderSystems line");
        return false;
    }
    // Trim trailing whitespace only (preserve leading spaces for format
    // checking)
    line.erase(line.find_last_not_of(" \t\r\n") + 1);
    const std::string expectedRenderSystemsPrefix = "RenderSystems:";
    if (line.substr(0, expectedRenderSystemsPrefix.size()) !=
        expectedRenderSystemsPrefix) {
        DixLogErr("Expected '{}', got: '{}'", expectedRenderSystemsPrefix,
                  line);
        return false;
    }
    size_t numRenderSystems =
        std::stoul(line.substr(expectedRenderSystemsPrefix.size() + 1));

    // Read each render system's objects
    for (size_t i = 0; i < numRenderSystems; ++i) {
        // Read render system name
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read RenderSystem line {}", i);
            return false;
        }
        // Only trim trailing whitespace, preserve leading spaces for format
        // checking
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedRSPrefix = "  RenderSystem: ";
        if (line.substr(0, expectedRSPrefix.size()) != expectedRSPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedRSPrefix, line);
            return false;
        }
        std::string renderSystemName = line.substr(expectedRSPrefix.size());

        // Read object count
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read ObjectCount line");
            return false;
        }
        // Only trim trailing whitespace, preserve leading spaces for format
        // checking
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedOCPrefix = "    ObjectCount:";
        if (line.substr(0, expectedOCPrefix.size()) != expectedOCPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedOCPrefix, line);
            return false;
        }
        size_t numObjects =
            std::stoul(line.substr(expectedOCPrefix.size() + 1));

        // Read each object
        auto& objects =
            m_initialStructure.gameObjectsByRenderSystem[renderSystemName];
        objects.reserve(numObjects);

        for (size_t j = 0; j < numObjects; ++j) {
            RecordedGameObject recorded;

            // Read object line: "    Object[j]: ID=xxx Model=yyy"
            if (!std::getline(m_inputStream, line)) {
                DixLogErr("Failed to read Object[{}] line", j);
                return false;
            }
            // Only trim trailing whitespace, preserve leading spaces for format
            // checking
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            const std::string expectedObjPrefix = "    Object[";
            if (line.substr(0, expectedObjPrefix.size()) != expectedObjPrefix) {
                DixLogErr("Expected '{}', got: '{}'", expectedObjPrefix, line);
                return false;
            }
            // Parse ID from the line
            size_t idPos = line.find("ID=");
            size_t modelPos = line.find("Model=");
            if (idPos == std::string::npos || modelPos == std::string::npos) {
                DixLogErr("Failed to parse ID or Model from: '{}'", line);
                return false;
            }
            recorded.id =
                std::stoull(line.substr(idPos + 3, modelPos - idPos - 4));
            recorded.modelPath = line.substr(modelPos + 6);

            // Read translation
            if (!std::getline(m_inputStream, line)) {
                DixLogErr("Failed to read Translation line");
                return false;
            }
            // Only trim trailing whitespace, preserve leading spaces for format
            // checking
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            const std::string expectedTransPrefix = "      Translation: ";
            if (line.substr(0, expectedTransPrefix.size()) !=
                expectedTransPrefix) {
                DixLogErr("Expected '{}', got: '{}'", expectedTransPrefix,
                          line);
                return false;
            }
            std::istringstream transStream(
                line.substr(expectedTransPrefix.size()));
            transStream >> recorded.translation.x >> recorded.translation.y >>
                recorded.translation.z;

            // Read rotation
            if (!std::getline(m_inputStream, line)) {
                DixLogErr("Failed to read Rotation line");
                return false;
            }
            // Only trim trailing whitespace, preserve leading spaces for format
            // checking
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            const std::string expectedRotPrefix = "      Rotation: ";
            if (line.substr(0, expectedRotPrefix.size()) != expectedRotPrefix) {
                DixLogErr("Expected '{}', got: '{}'", expectedRotPrefix, line);
                return false;
            }
            std::istringstream rotStream(line.substr(expectedRotPrefix.size()));
            rotStream >> recorded.rotation.x >> recorded.rotation.y >>
                recorded.rotation.z;

            // Read scale
            if (!std::getline(m_inputStream, line)) {
                DixLogErr("Failed to read Scale line");
                return false;
            }
            // Only trim trailing whitespace, preserve leading spaces for format
            // checking
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            const std::string expectedScalePrefix = "      Scale: ";
            if (line.substr(0, expectedScalePrefix.size()) !=
                expectedScalePrefix) {
                DixLogErr("Expected '{}', got: '{}'", expectedScalePrefix,
                          line);
                return false;
            }
            std::istringstream scaleStream(
                line.substr(expectedScalePrefix.size()));
            scaleStream >> recorded.scale.x >> recorded.scale.y >>
                recorded.scale.z;

            objects.push_back(recorded);

            // Build ID-to-location mapping for playback
            m_idToLocationMap[recorded.id] = {renderSystemName, j};
        }
    }

    // Read and skip frame separator
    if (!std::getline(m_inputStream, line)) {
        DixLogErr("Failed to read FRAMES_START line");
        return false;
    }
    line.erase(line.find_last_not_of(" \t\r\n") + 1);
    if (line != "FRAMES_START") {
        DixLogErr("Expected 'FRAMES_START', got: '{}'", line);
        return false;
    }

    // Read total frame count
    if (!std::getline(m_inputStream, line)) {
        DixLogErr("Failed to read TotalFrames line");
        return false;
    }
    line.erase(line.find_last_not_of(" \t\r\n") + 1);
    const std::string expectedTFPrefix = "TotalFrames:";
    if (line.substr(0, expectedTFPrefix.size()) != expectedTFPrefix) {
        DixLogErr("Expected '{}', got: '{}'", expectedTFPrefix, line);
        return false;
    }
    size_t numFrames = std::stoul(line.substr(expectedTFPrefix.size() + 1));

    // Read all frames into memory for easier playback
    m_frames.reserve(numFrames);
    for (size_t i = 0; i < numFrames; ++i) {
        RecordedFrame frame;

        // Read frame header: "Frame: N"
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read Frame {} header", i);
            return false;
        }
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedFramePrefix = "Frame: ";
        if (line.substr(0, expectedFramePrefix.size()) != expectedFramePrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedFramePrefix, line);
            return false;
        }
        // Frame number is parsed but not used since we read sequentially

        // Read frame time
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read FrameTime");
            return false;
        }
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedFTPrefix = "  FrameTime: ";
        if (line.substr(0, expectedFTPrefix.size()) != expectedFTPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedFTPrefix, line);
            return false;
        }
        frame.frameTime = std::stof(line.substr(expectedFTPrefix.size()));

        // Read camera position
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read CameraPosition");
            return false;
        }
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedCPPrefix = "  CameraPosition: ";
        if (line.substr(0, expectedCPPrefix.size()) != expectedCPPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedCPPrefix, line);
            return false;
        }
        std::istringstream camPosStream(line.substr(expectedCPPrefix.size()));
        camPosStream >> frame.cameraPosition.x >> frame.cameraPosition.y >>
            frame.cameraPosition.z;

        // Read camera look at
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read CameraLookAt");
            return false;
        }
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedCLAPrefix = "  CameraLookAt: ";
        if (line.substr(0, expectedCLAPrefix.size()) != expectedCLAPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedCLAPrefix, line);
            return false;
        }
        std::istringstream camLookStream(line.substr(expectedCLAPrefix.size()));
        camLookStream >> frame.cameraLookAt.x >> frame.cameraLookAt.y >>
            frame.cameraLookAt.z;

        // Read number of changed objects
        if (!std::getline(m_inputStream, line)) {
            DixLogErr("Failed to read ChangedObjects count");
            return false;
        }
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        const std::string expectedCOPrefix = "  ChangedObjects: ";
        if (line.substr(0, expectedCOPrefix.size()) != expectedCOPrefix) {
            DixLogErr("Expected '{}', got: '{}'", expectedCOPrefix, line);
            return false;
        }
        size_t numChanged = std::stoul(line.substr(expectedCOPrefix.size()));

        // Read each changed object's data
        for (size_t j = 0; j < numChanged; ++j) {
            // Read object ID line
            if (!std::getline(m_inputStream, line)) {
                DixLogErr("Failed to read ObjectID line");
                return false;
            }
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            const std::string expectedOIDPrefix = "    ObjectID: ";
            if (line.substr(0, expectedOIDPrefix.size()) != expectedOIDPrefix) {
                DixLogErr("Expected '{}', got: '{}'", expectedOIDPrefix, line);
                return false;
            }
            GameObject::id_t id =
                std::stoull(line.substr(expectedOIDPrefix.size()));
            frame.changedObjectIds.push_back(id);

            // Try to read optional transform lines
            // Look ahead for Translation, Rotation, Scale
            std::streampos currentPos = m_inputStream.tellg();
            std::string nextLine;

            // Check for translation
            if (std::getline(m_inputStream, nextLine)) {
                std::string trimmed = nextLine;
                trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
                const std::string expectedTransPrefix2 = "      Translation: ";
                if (trimmed.substr(0, expectedTransPrefix2.size()) ==
                    expectedTransPrefix2) {
                    std::istringstream tStream(
                        trimmed.substr(expectedTransPrefix2.size()));
                    glm::vec3 translation;
                    tStream >> translation.x >> translation.y >> translation.z;
                    frame.translations[id] = translation;
                } else {
                    m_inputStream.seekg(currentPos);
                }
                currentPos = m_inputStream.tellg();
            }

            // Check for rotation
            if (std::getline(m_inputStream, nextLine)) {
                std::string trimmed = nextLine;
                trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
                const std::string expectedRotPrefix2 = "      Rotation: ";
                if (trimmed.substr(0, expectedRotPrefix2.size()) ==
                    expectedRotPrefix2) {
                    std::istringstream rStream(
                        trimmed.substr(expectedRotPrefix2.size()));
                    glm::vec3 rotation;
                    rStream >> rotation.x >> rotation.y >> rotation.z;
                    frame.rotations[id] = rotation;
                } else {
                    m_inputStream.seekg(currentPos);
                }
                currentPos = m_inputStream.tellg();
            }

            // Check for scale
            if (std::getline(m_inputStream, nextLine)) {
                std::string trimmed = nextLine;
                trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
                const std::string expectedScalePrefix2 = "      Scale: ";
                if (trimmed.substr(0, expectedScalePrefix2.size()) ==
                    expectedScalePrefix2) {
                    std::istringstream sStream(
                        trimmed.substr(expectedScalePrefix2.size()));
                    glm::vec3 scale;
                    sStream >> scale.x >> scale.y >> scale.z;
                    frame.scales[id] = scale;
                } else {
                    m_inputStream.seekg(currentPos);
                }
            }
        }

        m_frames.push_back(std::move(frame));
    }

    return true;
}

}  // namespace dix