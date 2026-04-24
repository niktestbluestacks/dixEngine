#ifndef LOGGER_HPP
#define LOGGER_HPP

// std
#include <string>
#include <format>

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
}	// namespace dix

#endif // LOGGER_HPP