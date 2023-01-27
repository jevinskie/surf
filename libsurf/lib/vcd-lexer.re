#include "vcd-lexer.h"

namespace surf {

VCDLexer::VCDLexer() {
}

void VCDLexer::YYDEBUG(int state, char input) {
    m_state = state;
    m_input = input;
}

concurrencpp::generator<const char *> VCDLexer::parse(const char *vcd_cstr) {
    auto YYCURSOR = vcd_cstr;
    int count = 0;

    for (;;) {
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        *      { fmt::print("*\n"); co_yield YYCURSOR; }
        [\x00] { fmt::print("\\0\n"); co_return; }
        [a-z]+ { fmt::print("[a-z]+\n"); ++count; continue; }
        [ ]+   { fmt::print("space\n"); continue; }
    */
    }

}

};
