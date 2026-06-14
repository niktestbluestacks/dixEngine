#ifndef INPUT_BASE_HPP
#define INPUT_BASE_HPP

// dix
#include <Model/GameObject/GameObject.hpp>
#include <Window/WindowClass/WindowClass.hpp>

namespace dix {
class InputBase {
   public:
    struct KeyMappings {
        int moveLeft;
        int moveRight;
        int moveForward;
        int moveBackward;
        int moveUp;
        int moveDown;
        int lookLeft;
        int lookRight;
        int lookUp;
        int lookDown;
        int speedUp;
    };

    InputBase(Window& dixWindow, KeyMappings mappings)
        : m_window{dixWindow}, keys{mappings} {}

    virtual ~InputBase() = default;
    
    virtual void moveInPlaneXZ(float dt, GameObject& gameObject);

    KeyMappings keys;
    float moveSpeed{3.f};
    float lookSpeed{1.5f};

   protected:
    Window& m_window;
};
}  // namespace dix

#endif  // INPUT_BASE_HPP