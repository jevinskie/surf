#include <surf/vcd.h>

#include "common-internal.h"
#include "utils.h"
#include "vcd-lexer.h"
#include "vcd-parser.h"

VCDFile::VCDFile(const fs::path &path)
    : m_parsed_changes(false), m_mapped_file(path, (const void *)0x80'0000'0000) {
    fmt::print("vcd sz: {:d} data: {:p}\n", size(), fmt::ptr(data()));
    parse_declarations();
}

void VCDFile::parse_declarations() {
    const auto res          = parse_vcd_declarations(data());
    m_document.declarations = std::move(res.decls);
    m_sim_cmds              = res.remaining;
}

const VCDTypes::Document &VCDFile::document() {
    if (!m_parsed_changes) {
        m_document.sim_cmds = parse_vcd_sim_cmds(m_sim_cmds);
        m_parsed_changes    = true;
    }
    return m_document;
}

const VCDTypes::Declarations &VCDFile::declarations() const {
    return m_document.declarations;
}

const std::vector<VCDTypes::SimCmd> &VCDFile::sim_cmds() {
    if (!m_parsed_changes) {
        m_document.sim_cmds = parse_vcd_sim_cmds(m_sim_cmds);
        m_parsed_changes    = true;
    }
    return m_document.sim_cmds;
}

const char *VCDFile::data() const {
    return (const char *)m_mapped_file.data();
}

size_t VCDFile::size() const {
    return m_mapped_file.size();
}

std::shared_ptr<Trace> VCDFile::surf_trace() const {
    return m_trace;
}

int VCDFile::timebase_power() const {
    return m_trace->timebase_power();
}

Time VCDFile::start() const {
    return m_start;
}

Time VCDFile::end() const {
    return m_end;
}

// TODO: huge pages on linux
