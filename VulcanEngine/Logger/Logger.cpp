// dix
#include <Logger/Logger.hpp>

// std
#include <iostream>
#include <sstream>

namespace dix {

Logger& Logger::get() {
	static Logger instance;
	return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::ostringstream oss;
	oss << "[DIX ";
    switch (level) {
    case DEBUG: oss << "DEBUG]: "; break;
    case INFO: oss << "INFO]: "; break;
    case WARN:  oss << "WARN]: "; break;
    case ERR: oss << "ERROR]: "; break;
    }
    oss << message;

    switch(level) {
        case DEBUG:
        case INFO:
            std::clog << oss.str() << std::endl;
            break;
        case WARN:
        case ERR:
            std::cerr << oss.str() << std::endl;
            break;
    }
}

}	// namespace dix