#pragma once

#include <surf/vcd.h>

namespace surf {

struct VCDParserDeclRet {
    VCDTypes::Declarations decls;
    const char *remaining;
};
VCDParserDeclRet parse_vcd_declarations(const char *decls_cstr);
std::vector<VCDTypes::SimCmd> parse_vcd_sim_cmds(const char *sim_cmds_cstr);
VCDTypes::Document parse_vcd_document(const char *vcd_cstr);

}; // namespace surf
