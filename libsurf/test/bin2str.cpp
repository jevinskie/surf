#include <arm_neon.h>
#include <bitset>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include <surf/surf.h>

#include <fmt/format.h>

#define MCA_BEGIN(name)                                                                            \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-BEGIN " #name ::: "memory");                                    \
    } while (0)
#define MCA_END(name)                                                                              \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-END " #name ::: "memory");                                      \
    } while (0)

template <> struct fmt::formatter<uint8x16_t> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(uint8x16_t const &v8_16, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(
            ctx.out(),
            "<uint8x16_t {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} "
            "{:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x} {:#04x}>",
            v8_16[0], v8_16[1], v8_16[2], v8_16[3], v8_16[4], v8_16[5], v8_16[6], v8_16[7],
            v8_16[8], v8_16[9], v8_16[10], v8_16[11], v8_16[12], v8_16[13], v8_16[14], v8_16[15]);
    }
};

// correct
std::string ba2s(const std::span<const uint8_t> &packed_bits) {
    std::string result;
    result.reserve(packed_bits.size() * 8);
    for (std::size_t i = 0; i < packed_bits.size(); ++i) {
        uint8_t packed = packed_bits[i];
        for (int j = 7; j >= 0; --j) {
            uint8_t bit = (packed >> j) & 0x01;
            result.push_back(bit ? '1' : '0');
        }
    }
    return result;
}

std::string ba2s(const std::vector<uint8_t> &packed_bits) {
    return ba2s(std::span{packed_bits.data(), packed_bits.data() + packed_bits.size()});
}

template <typename T> std::string ba2s(const T &packed_bits) {
    return ba2s(std::span{(uint8_t *)&packed_bits, (uint8_t *)(&packed_bits + 1)});
}

#if 1
#if 0
// jev's NEON
std::string ba2s_neon(const std::span<const uint8_t> &bits) {
    std::string result;
    result.reserve(bits.size() * 8);
    auto data                 = bits.data();
    ssize_t size              = (ssize_t)bits.size();
    ssize_t i                 = 0;
    const uint8x16_t zeros_16 = vdupq_n_u8(0);
    const uint8x16_t ones_16  = vdupq_n_u8(1);
    for (; i <= size - 16; i += 16) {
        uint8x16_t bv = vld1q_u8(data + i);
        fmt::print("bv: {}\n", bv);
        const uint8x16_t v0 = vsriq_n_u8(bv, zeros_16, 7);
        fmt::print("v0: {}\n", v0);
    }
    for (; i < size; ++i) {}
    return result;
}
#else
#define BA2S_NEON_UNROLL_COUNT 8
std::string ba2s_neon(const std::span<const uint8_t> &bits) {
    const auto data = bits.data();
    ssize_t size    = (ssize_t)bits.size();
    std::string bitstring;
    bitstring.reserve(size * 8);
    const uint64x2_t magic       = {0x8040201008040201ull, 0x8040201008040201ull};
    const uint64x2_t mask        = {0x8080808080808080ull, 0x8080808080808080ull};
    const uint64x2_t ascii_zeros = {0x3030303030303030ull, 0x3030303030303030ull};
    ssize_t i                    = 0;
    // #pragma clang loop unroll_count(BA2S_NEON_UNROLL_COUNT)
    for (; i <= size - sizeof(uint16_t); i += sizeof(uint16_t)) {
        // MCA_BEGIN("ba2s_neon_inner_loop");
        const uint64x2_t words               = *(uint64x2_t *)(data + i);
        const uint64x2_t ones_or_zeros       = ((magic * words) & mask) >> 7;
        const uint64x2_t ones_or_zeros_ascii = ones_or_zeros + ascii_zeros;
        const auto ts =
            std::string_view{(const char *)&ones_or_zeros_ascii, sizeof(ones_or_zeros_ascii)};
        bitstring += ts;
        // MCA_END("ba2s_neon_inner_loop");
    }
    for (; i < size; ++i) {
        uint8_t byte = data[i];
        for (int8_t j = 7; j >= 0; --j) {
            const auto bit = (byte >> j) & 0x01;
            bitstring.push_back(bit ? '1' : '0');
        }
    }
    return bitstring;
}

#endif

std::string ba2s_neon(const std::vector<uint8_t> &packed_bits) {
    return ba2s_neon(std::span{packed_bits.data(), packed_bits.data() + packed_bits.size()});
}

template <typename T> std::string ba2s_neon(const T &packed_bits) {
    fmt::print("packed_bits type: {}\n", surf::type_name<decltype(packed_bits)>());
    return ba2s_neon(std::span{(uint8_t *)&packed_bits, (uint8_t *)(&packed_bits + 1)});
}

#endif

#if 0
// correct
std::string bit_array_to_string(const std::vector<uint8_t>& packed_bits) {
    std::string result;
    result.reserve(packed_bits.size() * 8);
    for (std::size_t i = 0; i < packed_bits.size(); ++i) {
        char packed = packed_bits[i];
        for (int j = 7; j >= 0; --j) {
            uint8_t bit = (packed >> j) & 0x01;
            result.push_back(bit ? '1' : '0');
        }
    }
    return result;
}
#endif

#if 0
std::string bit_array_to_string(const std::vector<uint8_t>& packed_bits) {
    std::string result;
    for (const auto& packed_bit : packed_bits) {
        for (int i = 7; i >= 0; --i) {
            result += (packed_bit & (1 << i)) ? '1' : '0';
        }
    }
    return result;
}
#endif

#if 0
std::string bit_array_to_string(const std::vector<uint8_t>& bits) {
    std::string result;
    const uint8_t* data = bits.data();
    size_t size = bits.size();
    size_t i = 0;
    for (; i <= size - 16; i += 16) {
        uint8x16_t bits_vector = vld1q_u8(data + i);
        uint8x16_t lookup_vector = vcreate_u8(0x0101010101010101ull);
        uint8x16_t result_vector = vtstq_u8(bits_vector, lookup_vector);
        uint8_t mask = 0x80;
        uint8x16_t mask_vector = vdupq_n_u8(mask);
        uint8x16_t mask_vector_2 = vorrq_u8(mask_vector, vshlq_n_u8(mask_vector, 1));
        uint8x16_t mask_vector_4 = vorrq_u8(mask_vector_2, vshlq_n_u8(mask_vector_2, 2));
        uint8x16_t mask_vector_8 = vorrq_u8(mask_vector_4, vshlq_n_u8(mask_vector_4, 4));
        result_vector = vandq_u8(result_vector, mask_vector_8);
        uint8_t result_array[16];
        vst1q_u8(result_array, result_vector);
        for (int j = 0; j < 16; ++j) {
            result += (result_array[j] & 0x80) ? '1' : '0';
            result += (result_array[j] & 0x40) ? '1' : '0';
            result += (result_array[j] & 0x20) ? '1' : '0';
            result += (result_array[j] & 0x10) ? '1' : '0';
            result += (result_array[j] & 0x08) ? '1' : '0';
            result += (result_array[j] & 0x04) ? '1' : '0';
            result += (result_array[j] & 0x02) ? '1' : '0';
            result += (result_array[j] & 0x01) ? '1' : '0';
        }
    }
    for (; i < size; ++i) {
        uint8_t byte = data[i];
        result += (byte & 0x80) ? '1' : '0';
        result += (byte & 0x40) ? '1' : '0';
        result += (byte & 0x20) ? '1' : '0';
        result += (byte & 0x10) ? '1' : '0';
        result += (byte & 0x08) ? '1' : '0';
        result += (byte & 0x04) ? '1' : '0';
        result += (byte & 0x02) ? '1' : '0';
        result += (byte & 0x01) ? '1' : '0';
    }
    return result;
}
#endif

int main(int argc, const char **argv) {
    assert(argc == 2);
    uint64_t n = std::stoull(argv[1], nullptr, 0);
    // fmt::print("ba2s: {}\n", ba2s(n));
    // fmt::print("ba2s_neon: {}\n", ba2s_neon(n));
    const std::vector<uint8_t> v16 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    fmt::print("ba2s: {}\n", ba2s(v16));
    fmt::print("ba2s_neon: {}\n", ba2s_neon(v16));
    return 0;
}
