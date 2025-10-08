#include <surf/vcd.h>

#include <mutex>

#include "common-internal.h"
#include "utils.h"
#include "vcd-parser.h"

VCDFile::VCDFile(const fs::path &path) : m_mapped_file(path) {
    fmt::print("vcd sz: {:d} data: {:p}\n", size(), fmt::ptr(data()));
    auto decls_ret          = parse_vcd_declarations(string_view(), m_mapped_file.path());
    m_document.declarations = decls_from_decl_list(std::move(decls_ret.decls));
    m_sim_cmds_str          = decls_ret.remaining;
}

const VCDTypes::Document &VCDFile::document() {
    std::call_once(m_parse_once_flag, [&] {
        m_document.sim_cmds = parse_vcd_sim_cmds(m_sim_cmds_str, m_mapped_file.path());
    });
    return m_document;
}

const VCDTypes::Declarations &VCDFile::declarations() const {
    return m_document.declarations;
}

void VCDFile::parse_test() {
    (void)document();
    // parse_vcd_document_test(string_view(), path());
}

const std::vector<VCDTypes::SimCmd> &VCDFile::sim_cmds() {
    document();
    return m_document.sim_cmds;
}

const fs::path &VCDFile::path() const {
    return m_mapped_file.path();
}

const char *VCDFile::data() const {
    return (const char *)m_mapped_file.data();
}

size_t VCDFile::size() const {
    return m_mapped_file.size();
}

std::string_view VCDFile::string_view() const {
    return m_mapped_file.string_view();
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
