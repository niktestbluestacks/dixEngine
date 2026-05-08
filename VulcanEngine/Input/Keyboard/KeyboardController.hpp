#ifndef KEYBOARD_CONTROLLER_HPP
#define KEYBOARD_CONTROLLER_HPP

// dix
#include <Model/GameObject/GameObject.hpp>
#include <Window/WindowClass/WindowClass.hpp>

namespace dix {
class KeyboardController {
public:
	struct KeyMappings {
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_SPACE;
		int moveDown = GLFW_KEY_LEFT_CONTROL;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;
		int speedUp = GLFW_KEY_LEFT_SHIFT;
	};

	void modeInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);

	KeyMappings keys{};
	float moveSpeed{ 3.f };
	float lookSpeed{ 1.5f };
    // mouse look state
	bool mouseCaptured{ false };
	double lastMouseX{ 0.0 };
	double lastMouseY{ 0.0 };
	bool firstMouse{ true };

	constexpr static float minMoveSpeed{ 3.f };
	constexpr static float maxMoveSpeed{ 10.f };
	constexpr static float AccelerationCoefficient{ 5.f };
};
}	// namespace dix
#endif // KEYBOARD_CONTROLLER_HPP
