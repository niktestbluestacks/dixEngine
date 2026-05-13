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
	glm::vec3 translation{}; // (position offset)
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation{};

	glm::mat4 mat4();

	glm::mat3 normalMatrix();
};

class GameObject {
public:
	using id_t = unsigned int;

	static GameObject createGameObject() {
		static id_t currentId = 0;
		return GameObject{ currentId++ };
	}

	DIX_DISABLE_COPY(GameObject)
	DIX_ENABLE_MOVE(GameObject)
	~GameObject() = default;

	id_t getId() const { return id; };

	std::shared_ptr <Model> model{};
	glm::vec3 color{};
	TransformComponent transform{};
private:
	GameObject(id_t objId) : id{ objId } {};

	id_t id;
};
}	// namespace dix

namespace std {
template <>
struct hash<dix::GameObject> {
size_t operator()(dix::GameObject gameObj) {
	size_t seed = 0;
	dix::hashCombine(seed, gameObj.getId());
	return seed;
}
};
}	// namespace std
#endif // GAME_OBJECT_HPP
