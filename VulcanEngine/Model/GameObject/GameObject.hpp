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

// Base class
class GameObject {
   public:
    using id_t = unsigned int;

    template <typename T, typename... Args>
    static T create(Args&&... args) {
        static id_t currentId = 0;

        return T{Key{}, currentId++, std::forward<Args>(args)...};
    }

    virtual ~GameObject() = default;

    id_t getId() const { return id; };

   protected:
    struct Key {
        friend class GameObject;
        explicit Key() = default;
    };

    GameObject(Key key, id_t objId) : id{objId} {};

    id_t id;

    template <typename T, typename... Args>
    friend T GameObject::create(Args&&... args);
};
}  // namespace dix

#endif  // GAME_OBJECT_HPP
