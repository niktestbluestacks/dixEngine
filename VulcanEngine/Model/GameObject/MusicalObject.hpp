// #ifndef MUSICAL_OBJECT_HPP
// #define MUSICAL_OBJECT_HPP

// // dix
// #include <Utils/Class.hpp>
// #include <Model/GameObject/GameObject.hpp>
// #include <Sound/DixAudio.hpp>

// namespace dix {
//     class MusicalObject {
//     public:
//         DIX_DISABLE_COPY(MusicalObject)
//         MusicalObject(MusicalObject&&) = default;
//         MusicalObject& operator=(MusicalObject&&) = default;

//         MusicalObject(const std::string& audioFilepath, GameObject&& gameObj);

//         GameObject& getGameObject() { return m_gameObject; }

//         void updatePosition(const glm::vec3& playerPos, const glm::vec3& lookingAt, const glm::vec3 upDir);
//         // for now it will always play in proximity
//     private:
//         GameObject m_gameObject;
//         DixAudio m_audio;
//     };
// }

// #endif // MUSICAL_OBJECT_HPP