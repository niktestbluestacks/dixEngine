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
        ui->setConsoleColor(&m_consoleColor);
    }
}

void DixConsole::log(const std::string& message) {
    m_history.push_back(message);
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.pop_front();
    }
}

void DixConsole::log(const std::vector<std::string>& message) {
    for (auto& elem : message) {
        m_history.push_back(elem);
        if (m_history.size() > MAX_HISTORY_SIZE) {
            m_history.pop_front();
        }
    }
}

void DixConsole::logDebug(const std::string& message) {
    log("[DEBUG] " + message);
}

void DixConsole::logDebug(const std::vector<std::string>& message) {
    log("[DEBUG]: ");
    log(message);
}

void DixConsole::logInfo(const std::string& message) {
    log("[INFO] " + message);
}

void DixConsole::logInfo(const std::vector<std::string>& message) {
    log("[INFO]: ");
    log(message);
}

void DixConsole::logWarn(const std::string& message) {
    log("[WARN] " + message);
}

void DixConsole::logWarn(const std::vector<std::string>& message) {
    log("[WARN]: ");
    log(message);
}

void DixConsole::logError(const std::string& message) {
    log("[ERROR] " + message);
}

void DixConsole::logError(const std::vector<std::string>& message) {
    log("[ERROR]: ");
    log(message);
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

// void DixConsole::arrowDown() {
//     if (m_isVisible) {
//         m_inputBuffer = m_history.back();
//     }
// }

// void DixConsole::arrowUp() {
//     if (m_isVisible) {
//         m_inputBuffer = m_history.back();
//     }
// }

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
            if (!found) {
                logError("Unknown command: " + commandName);
            }
            break;
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
    m_internalCommands["help"] = [this]() {
        log("help <-> helps you\n"
            "clear <-> clear the history\n"
            "particle_emitter {--bouncy} float x float y float z uint amount "
            "<->\n"
            "creates as particle emitter with or without bouncy property at x "
            "y z\n"
            "with amount particles\n"
            "echo acceps --debug e.t.c. <-> will print on new lines only after "
            "spaces\n"
            "exit <-> calls exit(EXIT_SUCCESS)\n"
            "termcolor accepts four floats or flags suck as --blue e.t.c. and "
            "float after it <->\n"
            "changes color of terminal (if -- flag then changes that color in "
            "terminal)");
    };

    m_internalCommands["clear"] = [this]() {
        m_history.clear();
        m_history.push_back(">");
    };

    m_internalCommands["exit"] = [this]() { std::exit(EXIT_SUCCESS); };

    m_internalCommandsWithArgs["termcolor"] =
        [this](const std::vector<std::string>& arguments) {
            glm::vec4 newColor = m_consoleColor;
            std::array<bool, 4> visited{};
            int nextToFill = -1;
            for (auto& elem : arguments) {
                auto res = string_to_num(elem);
                std::visit(
                    [&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, float>) {
                            if (nextToFill == -1) {
                                if (!visited[0]) {
                                    visited[0] = true;
                                    newColor[0] = std::clamp(arg, 0.f, 1.f);
                                } else if (!visited[1]) {
                                    visited[1] = true;
                                    newColor[1] = std::clamp(arg, 0.f, 1.f);
                                } else if (!visited[2]) {
                                    visited[2] = true;
                                    newColor[2] = std::clamp(arg, 0.f, 1.f);
                                } else if (!visited[3]) {
                                    visited[3] = true;
                                    newColor[3] = std::clamp(arg, 0.f, 1.f);
                                } else
                                    return;
                            } else {
                                visited[nextToFill] = true;
                                newColor[nextToFill] =
                                    std::clamp(arg, 0.f, 1.f);
                                nextToFill = -1;
                            }
                        } else if constexpr (std::is_same_v<T, std::string>) {
                            if (arg == "--red") {
                                nextToFill = 0;
                            } else if (arg == "--green") {
                                nextToFill = 1;
                            } else if (arg == "--blue") {
                                nextToFill = 2;
                            } else if (arg == "--alpha") {
                                nextToFill = 3;
                            }
                        }
                    },
                    res);
            }
            m_consoleColor = newColor;
        };

    m_internalCommandsWithArgs["echo"] = [this](const std::vector<std::string>&
                                                    arguments) {
        enum class FLAGS { Log, Debug, Info, Warn, Err };

        FLAGS flags = FLAGS::Log;

        std::string flag = arguments[0];
        if (flag == "--debug") {
            flags = FLAGS::Debug;
        } else if (flag == "--info") {
            flags = FLAGS::Info;
        } else if (flag == "--warn") {
            flags = FLAGS::Warn;
        } else if (flag == "--err") {
            flags = FLAGS::Err;
        }

        switch (flags) {
            case FLAGS::Log:
                log(arguments);
                break;
            case FLAGS::Debug:
                logDebug(std::vector(arguments.begin() + 1, arguments.end()));
                break;
            case FLAGS::Info:
                logInfo(std::vector(arguments.begin() + 1, arguments.end()));
                break;
            case FLAGS::Warn:
                logWarn(std::vector(arguments.begin() + 1, arguments.end()));
                break;
            case FLAGS::Err:
                logError(std::vector(arguments.begin() + 1, arguments.end()));
                break;
        }
    };
}

}  // namespace dix
