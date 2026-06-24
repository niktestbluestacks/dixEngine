// dix
#include <Input/Keyboard/KeyboardAndMouseController.hpp>
#include <Logger/Logger.hpp>

namespace dix {

void KeyboardAndMouseController::moveInPlaneXZ(float dt,
                                               GameObject& gameObj) {
    
    SimpleGameObject& gameObject = static_cast<SimpleGameObject&>(gameObj);
    glm::vec3 rotate{0.f};
    if (m_window.isKeyPressedUsual(keys.lookRight)) rotate.y += 1.f;
    if (m_window.isKeyPressedUsual(keys.lookLeft)) rotate.y -= 1.f;
    if (m_window.isKeyPressedUsual(keys.lookUp)) rotate.x -= 1.f;
    if (m_window.isKeyPressedUsual(keys.lookDown)) rotate.x += 1.f;

    if (m_window.isKeyPressedUsual(keys.speedUp)) {
        moveSpeed =
            std::min(maxMoveSpeed, moveSpeed + AccelerationCoefficient * dt);
    } else {
        moveSpeed =
            std::max(minMoveSpeed, moveSpeed - AccelerationCoefficient * dt);
    }

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.rotation +=
            lookSpeed * dt * glm::normalize(rotate);
    }

    gameObject.transform.rotation.x =
        glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y =
        glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{glm::sin(yaw), 0.f, glm::cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, 1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (m_window.isKeyPressedUsual(keys.moveForward)) moveDir += forwardDir;
    if (m_window.isKeyPressedUsual(keys.moveBackward)) moveDir -= forwardDir;
    if (m_window.isKeyPressedUsual(keys.moveRight)) moveDir += rightDir;
    if (m_window.isKeyPressedUsual(keys.moveLeft)) moveDir -= rightDir;
    if (m_window.isKeyPressedUsual(keys.moveUp)) moveDir += upDir;
    if (m_window.isKeyPressedUsual(keys.moveDown)) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.translation +=
            moveSpeed * dt * glm::normalize(moveDir);
    }

    // Mouse look: when right mouse button is pressed capture the cursor and use
    // relative mouse movement to adjust rotation. Release capture when button
    // released.
    if (m_window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!mouseCaptured) {
            mouseCaptured = true;
            firstMouse = true;
            m_window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        double xpos, ypos;
        m_window.getCursorPos(&xpos, &ypos);
        if (firstMouse) {
            lastMouseX = xpos;
            lastMouseY = ypos;
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos - lastMouseX);
        float yoffset = static_cast<float>(ypos - lastMouseY);
        lastMouseX = xpos;
        lastMouseY = ypos;

        // apply sensitivity and dt
        float sensitivity = lookSpeed * 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        gameObject.transform.rotation.y += glm::radians(xoffset);
        gameObject.transform.rotation.x += glm::radians(yoffset);

        gameObject.transform.rotation.x =
            glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y =
            glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
    } else {
        if (mouseCaptured) {
            mouseCaptured = false;
            m_window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
}  // namespace dix