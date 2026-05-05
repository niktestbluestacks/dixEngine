#ifndef DIX_TIME_COUNTER_HPP
#define DIX_TIME_COUNTER_HPP

// dix
#include <UI/DixUIElement.hpp>

// std
#include <chrono>

namespace dix {
namespace Time = std::chrono;
using Clock = std::chrono::system_clock;
class DixTimeCounter : public DixUIElement {
public:
    using DixUIElement::DixUIElement;
    DixTimeCounter(DixTimeCounter&& other) : 
        DixUIElement(std::move(other)),
        m_currTime(std::move(other.m_currTime)),
        m_startTime(std::move(other.m_startTime)) {};
    ~DixTimeCounter() override = default;

    void update(float dt) override;
private:
    Time::time_point<Clock> m_currTime;
    Time::system_clock::time_point m_startTime = std::chrono::system_clock::now();
};
}   // namespace dix

#endif // DIX_TIME_COUNTER_HPP
