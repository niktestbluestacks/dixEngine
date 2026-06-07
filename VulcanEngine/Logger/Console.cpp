// dix
#include <DixUI/DixConsoleUI.hpp>
#include <Logger/Console.hpp>

// std
#include <algorithm>
#include <span>
#include <sstream>

namespace dix {

void DixConsole::setPosition(float x1, float y1, float x2, float y2) {
    m_cornerPositions = glm::vec4(x1, y1, x2, y2);
    if (m_consoleUI_ptr) {
        m_consoleUI_ptr->setConsolePosition(m_cornerPositions);
    }
}

void DixConsole::setPosition(const glm::vec4& positions) {
    m_cornerPositions = positions;
    if (m_consoleUI_ptr) {
        m_consoleUI_ptr->setConsolePosition(positions);
    }
}

const glm::vec4& DixConsole::getPosition() const { return m_cornerPositions; }

void DixConsole::setConsoleUI(DixConsoleUI* ui) {
    m_consoleUI_ptr = ui;
    if (ui) {
        ui->setHistoryRef(&m_history);
        ui->setInputBufferCallback([this]() { return m_inputBuffer; });
        ui->setVisibilityCallback([this]() { return m_isVisible; });
        ui->setConsolePosition(m_cornerPositions);
    }
}

void DixConsole::log(const std::string& message) {
    m_history.push_back(message);
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.pop_front();
    }
}

void DixConsole::logDebug(const std::string& message) {
    log("[DEBUG] " + message);
}

void DixConsole::logInfo(const std::string& message) {
    log("[INFO] " + message);
}

void DixConsole::logWarn(const std::string& message) {
    log("[WARN] " + message);
}

void DixConsole::logError(const std::string& message) {
    log("[ERROR] " + message);
}

std::string DixConsole::getConsoleText() const {
    std::string result;
    for (const auto& line : m_history) {
        result += line + "\n";
    }
    return result;
}

void DixConsole::addCharacter(char c) {
    if (m_inputBuffer.size() < 256) {
        m_inputBuffer += c;
    }
}

void DixConsole::backspace() {
    if (!m_inputBuffer.empty()) {
        m_inputBuffer.pop_back();
    }
}

void DixConsole::enterCommand() {
    if (!m_inputBuffer.empty()) {
        executeCommand(m_inputBuffer);
        m_inputBuffer.clear();
    }
}

void DixConsole::executeCommandImpl(const std::vector<std::string>& args) {
    switch (args.size()) {
        case 0: {
            logError("No command provided");
            break;
        }
        case 1: {
            const std::string& commandName = args[0];
            bool found = false;

            if (m_internalCommands.contains(commandName)) {
                m_internalCommands[commandName]();
                found = true;
            } else if (m_commands.contains(commandName)) {
                m_commands[commandName]();
                found = true;
            }

        }

        default: {
            const std::string& commandName = args[0];
            bool found = false;
            auto res = std::vector(args.begin() + 1, args.end());
            if (m_internalCommandsWithArgs.contains(commandName)) {
                m_internalCommandsWithArgs[commandName](res);
                found = true;
            } else if (m_commandsWithArgs.contains(commandName)) {
                m_commandsWithArgs[commandName](res);
                found = true;
            }

            if (!found) {
                logError("Unknown command: " + commandName);
            }
            break;
        }
    }
}

void DixConsole::executeCommand(const std::string& command) {
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
    executeCommandImpl(args);
}

void DixConsole::fillInternalCommands() {
    m_internalCommands["clear"] = [this]() {
        m_history.clear();
        m_history.push_back(">");
    };
    m_internalCommands["help"] = [this]() {
        this->log("help <-> helps you\nclear <-> clear the history");
    };
}

}  // namespace dix
