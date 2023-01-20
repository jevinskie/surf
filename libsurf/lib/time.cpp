#include <surf/time.h>

#include "common-internal.h"

Time::Time() : m_timebase_power(0), m_ticks(0) {}

Time::Time(uint64_t ticks, int timebase_power) : m_timebase_power(timebase_power), m_ticks(ticks) {}

Time::Time(double seconds, int timebase_power)
    : m_timebase_power(timebase_power), m_ticks(seconds_to_ticks(seconds, m_timebase_power)) {}

uint64_t Time::ticks() const {
    return m_ticks;
}

double Time::seconds() const {
    return ticks_to_seconds(m_ticks, m_timebase_power);
}

int Time::timebase_power() const {
    return m_timebase_power;
}

uint64_t Time::seconds_to_ticks(double seconds, int timebase_power) {
    return 1;
}

double Time::ticks_to_seconds(uint64_t ticks, int timebase_power) {
    return 1;
}
