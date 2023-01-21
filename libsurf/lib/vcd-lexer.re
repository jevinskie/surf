#include "vcd-lexer.h"

namespace surf {

VCDLexer::VCDLexer() {
}

void VCDLexer::YYDEBUG(int state, char input) {
    m_state = state;
    m_input = input;
}

ssize_t VCDLexer::parse(const char *vcd_cstr) {
    auto YYCURSOR = vcd_cstr;
    int count = 0;

    for (;;) {
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        *      { return -1; }
        [\x00] { return count; }
        [a-z]+ { ++count; continue; }
        [ ]+   { continue; }
    */
    }

}

};
