#pragma once

#include "common-internal.h"

#include <concurrencpp/concurrencpp.h>

namespace surf {
class VCDLexer {
public:
    VCDLexer();
    concurrencpp::generator<const char *> parse(const char *vcd_cstr);

private:
    void YYDEBUG(int state, char input);
    int m_state;
    char m_input;
};

}; // namespace surf
