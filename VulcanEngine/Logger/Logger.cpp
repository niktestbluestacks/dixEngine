// dix
#include <Logger/Logger.hpp>

// std
#include <iostream>
#include <sstream>
#include <chrono>

#ifndef _WIN32
#define localtime_s(tm_ptr, time_ptr) localtime_r(time_ptr, tm_ptr)
#endif

namespace dix {

Logger& Logger::get() {
	static Logger instance;
	return instance;
}

void Logger::log(LogLevel level, const std::string message) {
    static auto start_time = std::chrono::system_clock::now();
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
            std::cerr << oss.str() << std::endl;
            break;
        case ERR:
            std::cerr << oss.str() << "\n";
            std::cerr << "The program had been running for " + std::to_string(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now() - start_time).count()
                ) + " seconds and had been termitated in: ";
            auto timeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm nowTmStorage;
            std::tm* nowTm = &nowTmStorage;
            localtime_s(nowTm, &timeT);
            std::cerr << std::to_string(nowTm->tm_year + 1900) + "-" +
                        std::to_string(nowTm->tm_mon + 1) + "-" +
                        std::to_string(nowTm->tm_mday) + "--" +
                        std::to_string(nowTm->tm_hour) + ":" + 
                        std::to_string(nowTm->tm_min) + ":" + 
                        std::to_string(nowTm->tm_sec) << std::endl;
            break;
    }
}

}	// namespace dix