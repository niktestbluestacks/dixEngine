// // dix
// #include <Model/GameObject/MusicalObject.hpp>

// namespace dix {

// MusicalObject::MusicalObject(const std::string& audioFilepath, GameObject&& gameObj) {
//     m_gameObject = std::move(gameObj);
//     m_audio = DixAudio(audioFilepath);
// }

// void MusicalObject::updatePosition(const glm::vec3& playerPos, const glm::vec3& lookingAt, const glm::vec3 upDir) {
//     m_audio.updateListener(playerPos, lookingAt, upDir);
// }
// }   // namespace dix