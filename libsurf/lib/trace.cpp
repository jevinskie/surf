#include <surf/trace.h>

#include "common-internal.h"
#include "utils.h"

Trace::Trace(const fs::path &path) : m_path(path) {
    m_fd = open(m_path.c_str(), O_RDONLY);
    posix_check(m_fd, "Surf trace open");
}

int Trace::timebase_power() const {
    return m_timebase_power;
}

Time Trace::start() const {
    return m_start;
}

Time Trace::end() const {
    return m_end;
}
