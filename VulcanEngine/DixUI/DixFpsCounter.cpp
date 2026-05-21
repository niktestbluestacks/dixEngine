#include <DixUI/DixFpsCounter.hpp>

namespace dix {

DixFpsCounter::DixFpsCounter(DixFpsCounter&& other) : DixUIElement(std::move(other)) {
        this->m_acc = other.m_acc;
        this->m_frames = other.m_frames;
        this->m_fps = other.m_fps;
        this->m_lastText = std::move(other.m_lastText);
}

void DixFpsCounter::update(float dt, const AdditionalUIInfo& additionalInfo) {
    m_acc += dt; 
    ++m_frames;
    if (m_acc >= 1.0f) {
        m_fps = static_cast<int>(std::round(static_cast<float>(m_frames) / m_acc));
        m_frames = 0; 
        m_acc = 0.f;
    }
    // rebuild vertex buffer when text changes. Do this during update so any staging
    // buffer uploads / copies happen outside of an active render pass.
    std::string text = std::to_string(m_fps) + " FPS";
    if (text != m_lastText) {
        buildVerticesForText(text, 8.f, 28.f);
        m_lastText = text;
    }
}
}   // namespace dix
