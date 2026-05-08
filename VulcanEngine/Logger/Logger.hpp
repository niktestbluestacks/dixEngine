#ifndef LOGGER_HPP
#define LOGGER_HPP

// std
#include <string_view>
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

#define DixLog(level, msg, ...) dix::Logger::get().log(level, msg __VA_OPT__(,)__VA_ARGS__)
#ifdef NDEBUG
	#define DixLogDebug(msg, ...)
#else
	#define DixLogDebug(msg, ...)\
	DixLog(dix::Logger::LogLevel::DEBUG, std::format(msg __VA_OPT__(,)__VA_ARGS__))
#endif
#define DixLogInfo(msg, ...) DixLog(dix::Logger::LogLevel::INFO, msg __VA_OPT__(,)__VA_ARGS__)
#define DixLogWarn(msg, ...) DixLog(dix::Logger::LogLevel::WARN, msg __VA_OPT__(,)__VA_ARGS__)
#define DixLogErr(msg, ...) DixLog(dix::Logger::LogLevel::ERR, msg __VA_OPT__(,)__VA_ARGS__)

#endif // LOGGER_HPP