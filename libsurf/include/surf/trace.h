#pragma once

#include "common.h"
#include "time.h"

namespace surf {

class SURF_EXPORT Trace {
public:
    Trace(const std::filesystem::path &path);
    int timebase_power() const;
    Time start() const;
    Time end() const;

private:
    const std::filesystem::path m_path;
    int m_fd;
    int m_timebase_power;
    Time m_start;
    Time m_end;
};

} // namespace surf
