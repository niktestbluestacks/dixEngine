// dix
#include <DixUI/DixConsoleUI.hpp>

// std
#include <algorithm>

namespace dix {

DixConsoleUI::DixConsoleUI(const DixUIInfo& info) : DixUIElement(info) {}

void DixConsoleUI::setConsolePosition(float x1, float y1, float x2, float y2) {
    m_consolePosition = glm::vec4(x1, y1, x2, y2);
}

void DixConsoleUI::setConsolePosition(const glm::vec4& coordinates) {
    m_consolePosition = coordinates;
}

const glm::vec4& DixConsoleUI::getConsolePosition() const {
    return m_consolePosition;
}

void DixConsoleUI::update(float dt, const AdditionalUIInfo& additionalInfo) {
    if (m_visibilityCallback && !m_visibilityCallback()) {
        clearVertices();
        return;
    }

    m_updateTimer += dt;

    if (m_updateTimer < 0.05f) {
        return;
    }
    m_updateTimer = 0.0f;

    if (m_historyRef && m_inputBufferCallback) {
        std::string displayText;
        size_t linesToShow = std::min(LINES_TO_DISPLAY, m_historyRef->size());

        if (linesToShow > 0) {
            auto it = m_historyRef->begin();
            std::advance(it, m_historyRef->size() - linesToShow);

            for (; it != m_historyRef->end(); ++it) {
                displayText += *it + "\n";
            }
        }

        displayText += "> " + m_inputBufferCallback();

        if (displayText != m_displayText || m_visibilityCallback()) {
            // IMPORTANT
            // ADD SOME BOOLEAN THAT WILL LOCK CONSOLLE IN CORNER
            buildVerticesForText(displayText, 0,
                                 additionalInfo.screenExtent.height);
            m_displayText = displayText;
        }
    }
}

}  // namespace dix
