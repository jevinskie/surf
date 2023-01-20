#pragma once

#include "common.h"
#include "time.h"
#include "trace.h"

namespace surf {

class SURF_EXPORT VCD {
public:
    VCD(const std::filesystem::path &path);
    std::shared_ptr<Trace> surf_trace() const;
    int timebase_power() const;
    Time start() const;
    Time end() const;

private:
    void parse();

    const std::filesystem::path m_path;
    int m_fd;
    std::shared_ptr<Trace> m_trace;
};

} // namespace surf
