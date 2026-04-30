#ifndef LOGGER_HPP
#define LOGGER_HPP

// std
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
}	// namespace dix

#define DixLogDebug(msg) dix::Logger::get().log(dix::Logger::LogLevel::DEBUG, msg)
#define DixLogInfo(msg) dix::Logger::get().log(dix::Logger::LogLevel::INFO, msg)
#define DixLogWarn(msg) dix::Logger::get().log(dix::Logger::LogLevel::WARN, msg)
#define DixLogErr(msg) dix::Logger::get().log(dix::Logger::LogLevel::ERR, msg)

#endif // LOGGER_HPP