// #ifndef FRAME_RECORDER_HPP
// #define FRAME_RECORDER_HPP

// // dix
// #include <Logger/Logger.hpp>
// #include <Model/GameObject/GameObject.hpp>

// // libs
// #include <glm/glm.hpp>

// // std
// #include <fstream>
// #include <map>
// #include <string>
// #include <unordered_map>
// #include <vector>

// namespace dix {

// struct RecordedGameObject {
//     GameObject::id_t id;
//     glm::vec3 translation;
//     glm::vec3 rotation;
//     glm::vec3 scale;
//     std::string modelPath;  // for initial structure
// };

// struct RecordedFrame {
//     float frameTime;
//     glm::vec3 cameraPosition;
//     glm::vec3 cameraLookAt;
//     std::vector<GameObject::id_t> changedObjectIds;
//     std::unordered_map<GameObject::id_t, glm::vec3> translations;
//     std::unordered_map<GameObject::id_t, glm::vec3> rotations;
//     std::unordered_map<GameObject::id_t, glm::vec3> scales;
// };

// struct InitialStructure {
//     // render system name -> vector of recorded game objects
//     std::unordered_map<std::string, std::vector<RecordedGameObject>>
//         gameObjectsByRenderSystem;
// };

// class FrameRecorder {
//    public:
//     FrameRecorder() = default;
//     ~FrameRecorder() = default;

//     DIX_DISABLE_COPY(FrameRecorder)
//     DIX_ENABLE_MOVE(FrameRecorder)

//     // Recording API
//     void startRecording(const std::string& filename);
//     void stopRecording();

//     void recordInitialStructure(
//         const std::unordered_map<std::string, std::vector<std::shared_ptr<GameObject>>>&
//             gameObjects);

//     void recordFrame(
//         const std::unordered_map<std::string, std::vector<std::shared_ptr<GameObject>>>&
//             gameObjects,
//         const glm::vec3& cameraPosition, const glm::vec3& cameraLookAt,
//         float frameTime);

//     // Playback API
//     void startPlayback(
//         const std::string& filename,
//         std::unordered_map<std::string, std::vector<std::shared_ptr<GameObject>>>& gameObjects,
//         glm::vec3& cameraPosition, glm::vec3& cameraLookAt);

//     void stopPlayback();

//     bool updatePlayback(
//         std::unordered_map<std::string, std::vector<std::shared_ptr<GameObject>>>& gameObjects,
//         glm::vec3& cameraPosition, glm::vec3& cameraLookAt);

//     const RecordedFrame* getCurrentFrame() const;

//    private:
//     bool m_isRecording = false;
//     bool m_isPlaying = false;
//     std::string m_filename;
//     std::ofstream m_outputStream;
//     std::ifstream m_inputStream;

//     InitialStructure m_initialStructure;
//     std::vector<RecordedFrame> m_frames;
//     size_t m_currentFrameIndex = 0;

//     // For recording: track previous state to detect changes
//     std::unordered_map<GameObject::id_t, glm::vec3> m_prevTranslations;
//     std::unordered_map<GameObject::id_t, glm::vec3> m_prevRotations;
//     std::unordered_map<GameObject::id_t, glm::vec3> m_prevScales;

//     // For playback: map (renderSystemName, objectIndex) -> recorded ID
//     std::map<std::pair<std::string, size_t>, GameObject::id_t> m_playbackIdMap;
//     // Reverse map: recorded ID -> (renderSystemName, objectIndex)
//     std::unordered_map<GameObject::id_t, std::pair<std::string, size_t>>
//         m_idToLocationMap;

//     // Helpers
//     std::string getModelPath(const GameObject& obj) const;
//     void writeInitialStructure();
//     void writeFrame(const RecordedFrame& frame, size_t frameNumber);
//     bool readInitialStructure();
//     bool readNextFrame(RecordedFrame& frame);
// };

// FrameRecorder& getFrameRecorder();

// }  // namespace dix

// #endif  // FRAME_RECORDER_HPP