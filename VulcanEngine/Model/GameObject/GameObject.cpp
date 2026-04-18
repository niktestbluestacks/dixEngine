#include <Model/GameObject/GameObject.hpp>

namespace dix {
glm::mat2 Transform2DComponent::mat2() {
	const float s = glm::sin(rotation);
	const float c = glm::cos(rotation);
	glm::mat2 rotMat{ {c, s}, {-s, c} };

	glm::mat2 scaleMat{ {scale.x, .0f, }, {.0f, scale.y} };
	return rotMat * scaleMat;
}

}