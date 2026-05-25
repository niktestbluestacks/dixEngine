// #ifndef FRAME_RECORDER_TPP
// #define FRAME_RECORDER_TPP

// template<>
// void FrameRecorder::recordFrame<std::unordered_map<std::string, std::vector <GameObject>>>(
//         float frameTime,
//     const glm::vec3& cameraPos,
//     const glm::vec3& cameraRot,
//     const std::unordered_map<std::string, std::vector <GameObject>>& gameObjects
// ) {
//     if (m_mode != Mode::Recording) {
//         return;
//     }

//     auto now = std::chrono::high_resolution_clock::now();
//     float timestamp = std::chrono::duration<float>(now - m_startTime).count();

//     // Capture all game object states
//     std::vector<GameObjectState> objectStates;
//     objectStates.reserve(gameObjects.size());
//     for (const auto& [renderSystemName, gameObjectContainer] : gameObjects) {

//         for (const auto& gameObject : gameObjectContainer) {
//             if (gameObject) {
//                 GameObjectState state{
//                     gameObject->getId(),
//                     gameObject->transform.translation,
//                     gameObject->transform.rotation,
//                     gameObject->transform.scale,
//                     gameObject->color
//                     renderSystemName
//                 };
//                 objectStates.push_back(state);
//             }
//         }
//     }

//     FrameRecord record{
//         frameTime,
//         timestamp,
//         cameraPos,
//         cameraRot,
//         static_cast<uint32_t>(objectStates.size()),
//         std::move(objectStates)
//     };

//     m_records.push_back(record);
//     m_currentFrame++;

//     // Auto-save every 100 frames
//     if (m_records.size() % 100 == 0) {
//         DixLogDebug("Recorded {} frames with {} objects...",
//         m_records.size(), m_records.back().objectCount);
//     }
// }

// #endif // FRAME_RECORDER_TPP