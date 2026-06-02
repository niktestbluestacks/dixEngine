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
        return GameObject{GameObject::getNewId()};
    }

    DIX_ENABLE_COPY(GameObject)
    GameObject(GameObject&& other) {
        model = other.model;
        color = other.color;
        transform = other.transform;
        id = getNewId();    
    }

    GameObject operator=(GameObject&& other) {
        model = other.model;
        color = other.color;
        transform = other.transform;
        id = getNewId();  
        return *this;
    }
    
    ~GameObject() = default;

    id_t getId() const { return id; };

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

   private:
    static id_t getNewId() {
        static id_t currentId = 0;
        return currentId++;
    }

    GameObject(id_t objId) : id{objId} {};

    id_t id;
};
}  // namespace dix

#endif  // GAME_OBJECT_HPP
