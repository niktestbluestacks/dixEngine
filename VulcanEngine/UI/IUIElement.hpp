#ifndef IUI_ELEMENT_HPP
#define IUI_ELEMENT_HPP

#include <Utils/FrameInfo.hpp>

namespace dix {
class IUIElement {
public:
    virtual ~IUIElement() = default;
    virtual void update(float dt) = 0;
    // update CPU-side state
    virtual void render(FrameInfo& fi) = 0;
    // upload GPU resources for the upcoming frame (called after beginFrame)
    virtual void upload(FrameInfo& fi) { (void)fi; }
};
}   // namespace dix

#endif // IUI_ELEMENT_HPP