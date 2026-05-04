#ifndef DIX_FPS_COUNTER_HPP
#define DIX_FPS_COUNTER_HPP

// dix
#include <UI/DixUIElement.hpp>

namespace dix {

class DixFpsCounter : public DixUIElement {
public:
    using DixUIElement::DixUIElement; // inherit constructor
    DixFpsCounter(DixFpsCounter&& other);
    ~DixFpsCounter() override = default;

    void update(float dt) override;

private:
    float m_acc = 0.f;
    int m_frames = 0;
    int m_fps = 0;
    std::string m_lastText;
};
}   // namwspace dix

#endif // DIX_FPS_COUNTER_HPP