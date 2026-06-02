#ifndef SIMPLE_GAME_OBJ_HPP
#define SIMPLE_GAME_OBJ_HPP

// dix
#include <Model/GameObject/GameObject.hpp>

namespace dix {

class SimpleGameObject : public GameObject {
   private:
    SimpleGameObject(GameObject::Key key, id_t id, std::shared_ptr<Model> model,
                     glm::vec3 color, TransformComponent transform)
        : GameObject{key, id},
          model{std::move(model)},
          color{std::move(color)},
          transform{std::move(transform)} {}
    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

    SimpleGameObject(GameObject::Key key, id_t id) : GameObject{key, id} {}

    friend class GameObject;

   public:
    std::shared_ptr<Model> getModel() { return model; }
    glm::vec3& getColor() { return color; }
    TransformComponent& getTransformComponent() { return transform; }
};

}  // namespace dix

#endif  // SIMPLE_GAME_OBJ_HPP