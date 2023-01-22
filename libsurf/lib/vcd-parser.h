#pragma once

#include <surf/vcd.h>

#include "common-internal.h"

namespace surf {

class VCDParser {
public:
    VCDParser();
    VCDTypes::Document parse_document(const char *vcd_cstr);
    VCDTypes::Declarations parse_declarations(const char *decls_cstr);
    std::vector<VCDTypes::Change> parse_changes(const char *changes_cstr);
};

}; // namespace surf
