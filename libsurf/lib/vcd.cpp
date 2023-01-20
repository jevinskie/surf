#include <surf/vcd.h>

#include "common-internal.h"
#include "utils.h"

VCD::VCD(const fs::path &path) : m_path(path) {
    m_fd = open(m_path.c_str(), O_RDONLY);
    posix_check(m_fd, "vcd open");
    parse();
}

void VCD::parse() {}

std::shared_ptr<Trace> VCD::surf_trace() const {
    return m_trace;
}

int VCD::timebase_power() const {
    return m_trace->timebase_power();
}

Time VCD::start() const {
    return m_trace->start();
}

Time VCD::end() const {
    return m_trace->end();
}

// TODO: huge pages on linux
