#ifndef ALL_LOGGING_HPP
#define ALL_LOGGING_HPP

#include <Logger/Logger.hpp>
#include <Logger/Console.hpp>

#define DixLogAll(msg, ...) \
DixLog(::dix::Logger::LogLevel::NONE, msg, ...); \
DixLogConsole(msg, ...);

#define DixLogInfoAll(msg, ...) \
DixLogInfo(msg, ...); \
DixLogInfoConsole(msg, ...);

#define DixLogDebugAll(msg, ...) \
DixLogDebug(msg, ...); \
DixLogDebugConsole(msg, ...);

#define DixLogWarnAll(msg, ...) \
DixLogWarn(msg, ...); \
DixLogWarnConsole(msg, ...);

#define DixLogErrAll(msg, ...) \
DixLogErr(msg, ...); \
DixLogErrConsole(msg, ...);

#endif // ALL_LOGGING_HPP