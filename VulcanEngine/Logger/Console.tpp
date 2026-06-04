#ifndef CONSOLE_TPP
#define CONSOLE_TPP

// dix
#include <DixUI/DixConsoleUI.hpp>
#include <Logger/Console.hpp>

// std
#include <algorithm>
#include <sstream>

namespace dix {

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::setPosition(float x1, float y1, float x2,
                                          float y2) {
    m_cornerPositions = glm::vec4(x1, y1, x2, y2);
    if (m_consoleUI_ptr) {
        m_consoleUI_ptr->setConsolePosition(m_cornerPositions);
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::setPosition(const glm::vec4& positions) {
    m_cornerPositions = positions;
    if (m_consoleUI_ptr) {
        m_consoleUI_ptr->setConsolePosition(positions);
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
const glm::vec4& DixConsole<Commands...>::getPosition() const {
    return m_cornerPositions;
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::setConsoleUI(DixConsoleUI* ui) {
    m_consoleUI_ptr = ui;
    if (ui) {
        ui->setHistoryRef(&m_history);
        ui->setInputBufferCallback([this]() { return m_inputBuffer; });
        ui->setVisibilityCallback([this]() { return m_isVisible; });
        ui->setConsolePosition(m_cornerPositions);
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::log(const std::string& message) {
    m_history.push_back(message);
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.pop_front();
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::logDebug(const std::string& message) {
    log("[DEBUG] " + message);
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::logInfo(const std::string& message) {
    log("[INFO] " + message);
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::logWarn(const std::string& message) {
    log("[WARN] " + message);
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::logError(const std::string& message) {
    log("[ERROR] " + message);
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
std::string DixConsole<Commands...>::getConsoleText() const {
    std::string result;
    for (const auto& line : m_history) {
        result += line + "\n";
    }
    return result;
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::addCharacter(char c) {
    if (m_inputBuffer.size() < 256) {
        m_inputBuffer += c;
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::backspace() {
    if (!m_inputBuffer.empty()) {
        m_inputBuffer.pop_back();
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::enterCommand() {
    if (!m_inputBuffer.empty()) {
        executeCommand(m_inputBuffer);
        m_inputBuffer.clear();
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
template <size_t... Indices>
void DixConsole<Commands...>::executeCommandImpl(
    const std::vector<std::string>& args, std::index_sequence<Indices...>) {
    if (args.empty()) {
        logError("No command provided");
        return;
    }

    const std::string& commandName = args[0];
    bool found = false;

    (([this, &commandName, &args, &found]() {
         auto& [name, func] = std::get<Indices>(m_commands);
         if (name == commandName) {
             found = true;
             try {
                 func();
             } catch (const std::exception& e) {
                 logError(std::string("Command execution failed: ") + e.what());
             }
         }
     }()),
     ...);

    if (m_internalCommands.contains(commandName)) {
        m_internalCommands[commandName]();
        found = true;
    }

    if (!found) {
        logError("Unknown command: " + commandName);
    }
}

template <typename... Commands>
    requires IsSupportedCommandType<Commands...>
void DixConsole<Commands...>::executeCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }

    std::istringstream iss(command);
    std::vector<std::string> args;
    std::string token;

    while (iss >> token) {
        args.push_back(token);
    }

    log("> " + command);
    executeCommandImpl(args, std::index_sequence_for<Commands...>{});
}

}  // namespace dix

#endif  // CONSOLE_TPP
