#pragma once

#include "common.h"
#include "mmap.h"
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
    const char *data() const;
    size_t size() const;

private:
    void parse();

    MappedReadOnlyFile m_mapped_file;
    Time m_start;
    Time m_end;
    std::shared_ptr<Trace> m_trace;
};

} // namespace surf
