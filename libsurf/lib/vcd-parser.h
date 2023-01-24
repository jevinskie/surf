#pragma once

#include <surf/vcd.h>

namespace surf {

struct VCDParserDeclRet {
    VCDTypes::Declarations decls;
    std::string_view remaining;
};
VCDParserDeclRet parse_vcd_declarations(std::string_view decls_str);
std::vector<VCDTypes::SimCmd> parse_vcd_sim_cmds(std::string_view sim_cmds_str);
VCDTypes::Document parse_vcd_document(std::string_view vcd_str, const std::filesystem::path &path);
void parse_vcd_document_test(std::string_view vcd_str, const std::filesystem::path &path);

}; // namespace surf
