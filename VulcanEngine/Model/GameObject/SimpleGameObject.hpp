#ifndef SIMPLE_GAME_OBJ_HPP
#define SIMPLE_GAME_OBJ_HPP

// dix
#include <Model/GameObject/GameObject.hpp>

namespace dix {

class SimpleGameObject : public GameObject {
   public:
    static SimpleGameObject createSimpleGameObject() {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock{mtx};
        return SimpleGameObject{};
    }

    ~SimpleGameObject() = default;
    SimpleGameObject(const SimpleGameObject& other)
        : GameObject{other},
          model{other.model},
          color{other.color},
          transform{other.transform} {}

    SimpleGameObject& operator=(const SimpleGameObject& other) {
        if (this != &other) {
            GameObject::operator=(other);
            this->color = other.color;
            this->model = other.model;
            this->transform = other.transform;
        }
        return *this;
    }

    SimpleGameObject(SimpleGameObject&& other) noexcept
        : GameObject{std::move(other)},
          model{std::move(other.model)},
          color{std::move(other.color)},
          transform{std::move(other.transform)} {}

    SimpleGameObject& operator=(SimpleGameObject&& other) noexcept {
        if (this != &other) {
            GameObject::operator=(std::move(other));
            this->model = std::move(other.model);
            this->color = std::move(other.color);
            this->transform = std::move(other.transform);
        }
        return *this;
    }

    std::shared_ptr<Model> model;
    glm::vec3 color;
    TransformComponent transform;
    private:
    using GameObject::GameObject;
    SimpleGameObject(): GameObject{}, model{}, color{}, transform{} {}
};

}  // namespace dix

#endif  // SIMPLE_GAME_OBJ_HPP