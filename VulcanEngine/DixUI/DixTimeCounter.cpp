#include <DixUI/DixTimeCounter.hpp>
#include <chrono>
#include <ctime>
#include <string>

#ifndef _WIN32
#define localtime_s(tm_ptr, time_ptr) localtime_r(time_ptr, tm_ptr)
#endif

namespace dix {
void DixTimeCounter::update(float dt, const AdditionalUIInfo& additionalInfo) {
    m_currTime = Clock::now();
    auto timeT = Time::system_clock::to_time_t(m_currTime);
    static std::string lastText = "";
    if (timeT - std::chrono::system_clock::to_time_t(m_startTime) > 0) {
        m_startTime = m_currTime;
        std::tm nowTmStorage;
        std::tm* nowTm = &nowTmStorage;
        localtime_s(nowTm, &timeT);
        lastText = std::to_string(nowTm->tm_year + 1900) + "-" +
                        std::to_string(nowTm->tm_mon + 1) + "-" +
                        std::to_string(nowTm->tm_mday) + "--" +
                        std::to_string(nowTm->tm_hour) + ":" + 
                        std::to_string(nowTm->tm_min) + ":" + 
                        std::to_string(nowTm->tm_sec);
        buildVerticesForText(lastText, 8.f, 4.f);
    }
}
}   // namespace dix