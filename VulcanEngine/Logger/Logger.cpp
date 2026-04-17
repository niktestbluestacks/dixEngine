#include <Logger/Logger.hpp>

#include <iostream>

namespace dix {

Logger& Logger::get() {
	static Logger instance;
	return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
	std::string levelStr = "[DIX ";
    switch (level) {
    case DEBUG: levelStr += "DEBUG]: "; break;
    case INFO:  levelStr += "INFO]: "; break;
    case WARN:  levelStr += "WARN]: "; break;
    case ERR: levelStr += "ERROR]: "; break;
    }

	std::clog << levelStr << message << std::endl;
}


}