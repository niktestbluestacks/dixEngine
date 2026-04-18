#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

// dix
#include <Model/Model.hpp>

// std
#include <memory>

namespace dix {
struct Transform2DComponent {
	glm::vec2 translation{}; // (position offset)
	glm::vec2 scale{ 1.f, 1.f };
	float rotation;

	glm::mat2 mat2();
};

class GameObject {
public:
	using id_t = unsigned int;

	static GameObject createGameObject() {
		static id_t currentId = 0;
		return GameObject{ currentId++ };
	}

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	id_t getId() const { return id; };

	std::shared_ptr <Model> model{};
	glm::vec3 color{};
	Transform2DComponent transform2d{};
private:
	GameObject(id_t objId) : id{ objId } {};

	id_t id;
};
}	// namespace dix
#endif // GAME_OBJECT_HPP
