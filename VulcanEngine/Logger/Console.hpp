#ifndef CONSOLE_HPP
#define CONSOLE_HPP

// dix
#include <UI/DixUIElement.hpp>
#include <Utils/DixConcepts.hpp>

// std
#include <tuple>

namespace dix {

template <typename... Args>
using DixCommandInputType = std::pair<std::string, std::function<void(Args...)>>;

template <typename... Commands>
requires IsSupportedCommandType<Commands...>
class DixConsole {
public:
    void setPosition(float x1, float y1, float x2, float y2);
    void setPosition(const glm::vec4& positions);
    const glm::vec4& getPosition();
private:
    std::vector<std::string> m_history;
    std::vector<std::string> m_commands;
    glm::vec4 m_cournerPositions;
};

}   // namespace dix

#include <Logger/Console.tpp>

#endif // CONSOLE_HPP