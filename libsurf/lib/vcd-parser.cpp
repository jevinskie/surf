#include "vcd-parser.h"

#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy/callback.hpp>     // value callbacks
#include <lexy/dsl.hpp>          // lexy::dsl::*

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

using namespace VCDTypes;

namespace surf {

namespace {

namespace grammar {
namespace dsl = lexy::dsl;

};

}; // namespace

VCDParser::VCDParser() {}

VCDTypes::Document VCDParser::parse_document(const char *vcd_cstr) {
    return {};
}

VCDTypes::Declarations VCDParser::parse_declarations(const char *decls_cstr) {
    return {};
}

std::vector<VCDTypes::Change> VCDParser::parse_changes(const char *changes_cstr) {
    return {};
}

}; // namespace surf
