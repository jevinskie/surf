#include <surf/surf.h>
using namespace surf;

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>

#define TS "[VarBit]"

TEST_CASE("build", TS) {
    uint64_t buf = 0b1010;
    VarBit vb{(uint8_t *)&buf, 4};
    fmt::print("VarBit: {}\n", vb);
}
