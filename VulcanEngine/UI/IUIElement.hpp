#pragma once

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
}
