#include <surf/varbit.h>

#include "common-internal.h"
#include "utils.h"

#include <arm_neon.h>

std::string bitview2string(bitview bv) {
    std::string bitstring;
    const auto data        = bv.data();
    const auto bitsize     = bv.bitsize();
    const ssize_t bytesize = (ssize_t)bv.bytesize();
    if (bitsize == 0) {
        return "";
    }
    if (bitsize == 1) {
        return (*bv.data() & 1) ? "1" : "0";
    }
    bitstring.reserve(bytesize * 8);
    const uint64x2_t magic       = {0x8040201008040201ull, 0x8040201008040201ull};
    const uint64x2_t mask        = {0x8080808080808080ull, 0x8080808080808080ull};
    const uint64x2_t ascii_zeros = {0x3030303030303030ull, 0x3030303030303030ull};
    ssize_t i                    = 0;
#pragma clang loop unroll_count(4)
    for (; i <= bytesize - (ssize_t)sizeof(uint16_t); i += sizeof(uint16_t)) {
        // MCA_BEGIN("bitview2string_inner_loop");
        const uint64x2_t words               = {*(uint8_t *)(data + i), *(uint8_t *)(data + i + 1)};
        const uint64x2_t ones_or_zeros       = ((magic * words) & mask) >> 7;
        const uint64x2_t ones_or_zeros_ascii = ones_or_zeros + ascii_zeros;
        const auto ts =
            std::string_view{(const char *)&ones_or_zeros_ascii, sizeof(ones_or_zeros_ascii)};
        bitstring += ts;
        // MCA_END("bitview2string_inner_loop");
    }
    for (; i < bytesize; ++i) {
        uint8_t byte = data[i];
        for (int8_t j = 7; j >= 0; --j) {
            const auto bit = (byte >> j) & 1;
            bitstring.push_back(bit ? '1' : '0');
        }
    }
    bitstring.erase(bitstring.begin(), bitstring.begin() + (bytesize * CHAR_BIT - bitsize));
    return bitstring;
}
