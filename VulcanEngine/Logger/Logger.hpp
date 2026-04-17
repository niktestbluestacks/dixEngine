#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

namespace dix {

class Logger {
public:
enum LogLevel {
	DEBUG,
	INFO,
	WARN,
	ERR
};

static Logger& get();

void log(LogLevel level, const std::string& message);
}; // class Logger
}

#endif // LOGGER_HPP