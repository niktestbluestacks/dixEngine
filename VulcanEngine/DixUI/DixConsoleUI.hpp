#ifndef DIX_CONSOLE_UI_HPP
#define DIX_CONSOLE_UI_HPP

// dix
#include <UI/DixUIElement.hpp>

// std
#include <deque>
#include <functional>
#include <string>

namespace dix {

class DixConsoleUI : public DixUIElement {
   public:
    DixConsoleUI(const DixUIInfo& info);
    ~DixConsoleUI() override = default;

    void setConsolePosition(float x1, float y1, float x2, float y2);
    void setConsolePosition(const glm::vec4& coordinates);
    const glm::vec4& getConsolePosition() const;

    void update(float dt, const AdditionalUIInfo& additionalInfo) override;

    void setHistoryRef(const std::deque<std::string>* history) {
        m_historyRef = history;
    }

    void setInputBufferCallback(std::function<std::string()> callback) {
        m_inputBufferCallback = callback;
    }

    void setVisibilityCallback(std::function<bool()> callback) {
        m_visibilityCallback = callback;
    }

    void setConsoleColor(const glm::vec4* consoleColor) {
        m_consoleColor = consoleColor;
    }

    static constexpr size_t LINES_TO_DISPLAY = 25;

   private:
    glm::vec4 m_consolePosition = {0.f, 0.f, 1.0f, 1.0f};
    const std::deque<std::string>* m_historyRef = nullptr;
    std::function<std::string()> m_inputBufferCallback;
    std::function<bool()> m_visibilityCallback;
    std::string m_displayText;
    float m_updateTimer = 0.0f;
    const glm::vec4* m_consoleColor = nullptr;
};

}  // namespace dix

#endif  // DIX_CONSOLE_UI_HPP