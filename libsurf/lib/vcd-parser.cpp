#include "vcd-parser.h"
#include "common-internal.h"

#include <boost/spirit/home/x3.hpp>

#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy/callback.hpp>     // value callbacks
#include <lexy/dsl.hpp>          // lexy::dsl::*

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

#include <concurrencpp/concurrencpp.h>

using namespace VCDTypes;

namespace surf {

namespace {

namespace grammar {
namespace dsl = lexy::dsl;

};

}; // namespace

VCDParserDeclRet parse_vcd_declarations(const char *decls_cstr) {
    return {};
}

std::vector<SimCmd> parse_vcd_sim_cmds(const char *sim_cmds_cstr) {
    std::vector<SimCmd> cmds;
    return cmds;
}

VCDTypes::Document parse_vcd_document(const char *vcd_cstr) {
    auto declsret = parse_vcd_declarations(vcd_cstr);
    return {.declarations = std::move(declsret.decls),
            .sim_cmds     = parse_vcd_sim_cmds(declsret.remaining)};
}

}; // namespace surf
