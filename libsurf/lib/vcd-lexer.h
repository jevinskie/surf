#pragma once

#include "common-internal.h"

namespace surf {
class VCDLexer {
public:
    VCDLexer();
    ssize_t parse(const char *vcd_cstr);

private:
    void YYDEBUG(int state, char input);
    int m_state;
    char m_input;
};

}; // namespace surf
