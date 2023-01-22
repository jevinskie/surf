#include <surf/vcd.h>

#include "common-internal.h"
#include "utils.h"
#include "vcd-lexer.h"
#include "vcd-parser.h"

VCDFile::VCDFile(const fs::path &path)
    : m_parsed_changes(false), m_mapped_file(path, (const void *)0x80'0000'0000) {
    parse();
}

VCDTypes::Document VCDFile::parse() {
    fmt::print("vcd sz: {:d} data: {:p}\n", size(), fmt::ptr(data()));
    // VCDLexer lexer;
    // const auto res = lexer.parse(data());
    // fmt::print("lexing res: {:d}\n", res);
    VCDParser parser;
    return parser.parse_document(data());
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
