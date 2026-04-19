#ifndef CAMERA_HPP
#define CAMERA_HPP

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace dix {
class Camera {
public:
	void setOrthographicProjection(
		float left, 
		float right, 
		float top, 
		float bottom, 
		float near, 
		float far);

	void setPerspectiveProjection(
		float fovy, 
		float aspect, 
		float near, 
		float far);

	const glm::mat4& getProjection(void) const { return m_projectionMatrix; }

private:
	glm::mat4 m_projectionMatrix{ 1.f };

};
}
#endif // CAMERA_HPP