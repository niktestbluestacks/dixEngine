#ifndef KEYBOARD_CONTROLLER_HPP
#define KEYBOARD_CONTROLLER_HPP

// dix
#include <Input/InputBase.hpp>

namespace dix {

class KeyboardAndMouseController : public InputBase {
   public:
    using InputBase::InputBase;
    KeyboardAndMouseController(Window& dixWindow)
        : InputBase{dixWindow,
                    {
                        .moveLeft = GLFW_KEY_A,
                        .moveRight = GLFW_KEY_D,
                        .moveForward = GLFW_KEY_W,
                        .moveBackward = GLFW_KEY_S,
                        .moveUp = GLFW_KEY_SPACE,
                        .moveDown = GLFW_KEY_LEFT_CONTROL,
                        .lookLeft = GLFW_KEY_LEFT,
                        .lookRight = GLFW_KEY_RIGHT,
                        .lookUp = GLFW_KEY_UP,
                        .lookDown = GLFW_KEY_DOWN,
                        .speedUp = GLFW_KEY_LEFT_SHIFT,
                    }} {}

    ~KeyboardAndMouseController() override = default;

    void moveInPlaneXZ(float dt, GameObject& gameObject) override;

    // mouse look state
    bool mouseCaptured{false};
    double lastMouseX{0.0};
    double lastMouseY{0.0};
    bool firstMouse{true};

    static constexpr float minMoveSpeed{3.f};
    static constexpr float maxMoveSpeed{10.f};
    static constexpr float AccelerationCoefficient{5.f};
};
}  // namespace dix
#endif  // KEYBOARD_CONTROLLER_HPP
