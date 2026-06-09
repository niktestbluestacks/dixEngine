#ifndef DIX_CONSOLE_HPP
#define DIX_CONSOLE_HPP

// dix
#include <DixUI/DixConsoleUI.hpp>
#include <Utils/Class.hpp>
#include <Utils/DixConcepts.hpp>

// std
#include <any>
#include <deque>
#include <functional>
#include <tuple>

namespace dix {

template <typename... Args>
using DixCommandInputType =
    std::pair<std::string, std::function<void(Args...)>>;

class DixConsole {
   public:
    DIX_DISABLE_COPY_AND_MOVE(DixConsole)

    static DixConsole& getDixConsole() {
        static DixConsole instance;
        return instance;
    }

    void setPosition(float x1, float y1, float x2, float y2);
    void setPosition(const glm::vec4& positions);
    const glm::vec4& getPosition() const;

    void log(const std::string& message);
    void log(const std::vector<std::string>& message);
    void logDebug(const std::string& message);
    void logDebug(const std::vector<std::string>& message);
    void logInfo(const std::string& message);
    void logInfo(const std::vector<std::string>& message);
    void logWarn(const std::string& message);
    void logWarn(const std::vector<std::string>& message);
    void logError(const std::string& message);
    void logError(const std::vector<std::string>& message);

    void executeCommand(const std::string& command);
    std::string getConsoleText() const;

    void setConsoleUI(DixConsoleUI* ui);
    DixConsoleUI* getConsoleUI() const { return m_consoleUI_ptr; }

    const std::deque<std::string>& getHistory() const { return m_history; }

    void toggleConsole() { m_isVisible = !m_isVisible; }
    bool isVisible() const { return m_isVisible; }

    void addCharacter(char c);
    void backspace();
    void enterCommand();
    // void arrowUp();
    // void arrowDown();

    const std::string& getInputBuffer() const { return m_inputBuffer; }

    void register_function(const std::string& name,
                           std::function<void()> func) {
        if (m_internalCommands.contains(name) || m_commands.contains(name)) {
            throw std::runtime_error("The command with name " + name +
                                     " already exists!");
        }
        m_commands[name] = func;
    }

    void register_function(DixCommandInputType<>& command) {
        if (m_internalCommands.contains(command.first) ||
            m_commands.contains(command.first)) {
            throw std::runtime_error("The command with name " + command.first +
                                     " already exists!");
        }
        m_commands[command.first] = command.second;
    }

    void register_function(
        const std::string& name,
        std::function<void(const std::vector<std::string>&)> func) {
        if (m_internalCommandsWithArgs.contains(name) ||
            m_commandsWithArgs.contains(name)) {
            throw std::runtime_error("The command with name " + name +
                                     " already exists!");
        }
        m_commandsWithArgs[name] = func;
    }

    void register_function(
        DixCommandInputType<const std::vector<std::string>&> command) {
        if (m_internalCommandsWithArgs.contains(command.first) ||
            m_commandsWithArgs.contains(command.first)) {
            throw std::runtime_error("The command with name " + command.first +
                                     " already exists!");
        }
        m_commandsWithArgs[command.first] = command.second;
    }

    static constexpr size_t MAX_HISTORY_SIZE = DixConsoleUI::LINES_TO_DISPLAY;

   private:
    DixConsole() { fillInternalCommands(); }

    std::deque<std::string> m_history;
    std::unordered_map<std::string, std::function<void()>> m_commands;
    std::unordered_map<std::string,
                       std::function<void(const std::vector<std::string>&)>>
        m_commandsWithArgs;
    std::unordered_map<std::string, std::function<void()>> m_internalCommands;
    std::unordered_map<std::string,
                       std::function<void(const std::vector<std::string>&)>>
        m_internalCommandsWithArgs;
    glm::vec4 m_cornerPositions = {0.f, 100.f, 100.f, 100.f};
    glm::vec4 m_consoleColor{0.06f, 0.06f, 0.45f, 1.f};
    DixConsoleUI* m_consoleUI_ptr = nullptr;

    bool m_isVisible = false;
    std::string m_inputBuffer;

    void executeCommandImpl(const std::vector<std::string>& args);

    void fillInternalCommands();
};

}  // namespace dix

#include <Logger/Console.tpp>

#endif  // DIX_CONSOLE_HPP