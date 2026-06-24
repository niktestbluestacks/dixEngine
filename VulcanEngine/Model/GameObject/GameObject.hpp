#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

// dix
#include <Model/Model.hpp>
#include <Utils/Class.hpp>
#include <Utils/Hash.hpp>

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <mutex>

namespace dix {
struct TransformComponent {
    glm::vec3 translation{};  // (position offset)
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};

    glm::mat4 mat4();

    glm::mat3 normalMatrix();
};

class GameObject {
   public:
    using id_t = unsigned int;

    static GameObject createGameObject() {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock{mtx};
        return GameObject{};
    }

    GameObject(const GameObject& other) = default;

    GameObject& operator=(const GameObject& other) = default;
    GameObject(GameObject&& other) noexcept = default;

    GameObject& operator=(GameObject&& other) noexcept {
        this->id = std::move(other.id);
        return *this;
    }

    virtual ~GameObject() = default;

    const id_t getId() const { return id; };

   protected:
    static id_t getNewId() {
        static id_t currentId = 0;
        return currentId++;
    }

    GameObject() : id{getNewId()} {}

   private:
    id_t id;
};
}  // namespace dix

namespace std {
template <>
struct hash<dix::GameObject> {
    size_t operator()(const dix::GameObject& obj) {
        return hash<unsigned int>()(obj.getId());
    }
};
}  // namespace std

#endif  // GAME_OBJECT_HPP
