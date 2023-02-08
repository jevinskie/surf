#include <surf/surf.h>
using namespace surf;

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>

#define TS "[VarBit]"

TEST_CASE("build", TS) {
    uint64_t buf  = 0b1010;
    const auto bv = bitview{buf, 4};
    VarBit vb{bv};
    fmt::print("VarBit: {}\n", vb);
    REQUIRE(fmt::format("{}", vb) == "<VarBit[4] 1010>");
}
