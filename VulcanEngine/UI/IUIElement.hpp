#pragma once

#include <Utils/FrameInfo.hpp>

namespace dix {
class IUIElement {
public:
    virtual ~IUIElement() = default;
    virtual void update(float dt) = 0;
    virtual void render(FrameInfo& fi) = 0;
};
}
