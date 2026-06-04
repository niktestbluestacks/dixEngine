#ifndef DIX_CONSOLE_UI_HPP
#define DIX_CONSOLE_UI_HPP

// dix
#include <UI/DixUIElement.hpp>

namespace dix {
class DixConsoleUI : public DixUIElement {
public:
    void setConsolePosition(float x1, float y1, float x2, float y2);
    void setConsolePosition(const glm::vec4& coordinates);
    const glm::vec4& getConsolePosition();
private:
    glm::vec4 m_consolePosition;
};
}   // namespace dix

#endif // DIX_CONSOLE_UI_HPP