#pragma once

#include "common-internal.h"
#include <surf/vcd.h>

#include <stdexcept>

#include <lexy/visualize.hpp> // lexy::visualization_options

namespace surf {

struct VCDDeclParseError : public std::invalid_argument {};
struct VCDSimCmdsParseError : public std::invalid_argument {};

struct VCDParserDeclRet {
    std::vector<VCDTypes::Declaration> decls;
    std::string_view remaining;
};
VCDParserDeclRet parse_vcd_declarations(std::string_view decls_str, fs::path = "unknown",
                                        std::optional<lexy::visualization_options> opts = {});

std::vector<VCDTypes::SimCmd>
parse_vcd_sim_cmds(std::string_view sim_cmds_str, fs::path = "unknown",
                   std::optional<lexy::visualization_options> opts = {});

VCDTypes::Document parse_vcd_document(std::string_view vcd_str, fs::path path = "unknown",
                                      std::optional<lexy::visualization_options> opts = {});

void parse_vcd_document_test(std::string_view vcd_str, const std::filesystem::path &path);

VCDTypes::Declarations decls_from_decl_list(std::vector<VCDTypes::Declaration> &decls);

}; // namespace surf
