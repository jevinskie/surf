#include <surf/vcd.h>

#include "common-internal.h"
#include "utils.h"

VCD::VCD(const fs::path &path) : m_mapped_file(path) {
    parse();
}

void VCD::parse() {}

const char *VCD::data() const {
    return (const char *)m_mapped_file.data();
}

size_t VCD::size() const {
    return m_mapped_file.size();
}

std::shared_ptr<Trace> VCD::surf_trace() const {
    return m_trace;
}

int VCD::timebase_power() const {
    return m_trace->timebase_power();
}

Time VCD::start() const {
    return m_start;
}

Time VCD::end() const {
    return m_end;
}

// TODO: huge pages on linux
