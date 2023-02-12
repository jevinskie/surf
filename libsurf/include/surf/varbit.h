#pragma once

#include "common.h"

#include <arm_neon.h>

// TODO: optimize/remove/#ifguard masking off of bits

namespace surf {

namespace varbit {
using sz_t = uint16_t;
}; // namespace varbit

class SURF_PACKED bitview {
public:
    bitview(const uint8_t *buf, varbit::sz_t bitsz) : m_buf{buf}, m_size{bitsz} {
        assert(bitsz > 0);
    }
    template <typename T>
    bitview(const T &buf, varbit::sz_t bitsz) : m_buf{(const uint8_t *)&buf}, m_size{bitsz} {
        assert(bitsz > 0 && bitsz <= sizeof(T) * CHAR_BIT);
    }

    const uint8_t *data() const {
        return m_buf;
    }
    varbit::sz_t bitsize() const {
        return m_size;
    }
    varbit::sz_t bytesize() const {
        return roundup_pow2_mul(m_size, 8) / 8;
    }

private:
    const uint8_t *m_buf;
    varbit::sz_t m_size;
};

class bitarray4 {
public:
    bitarray4(uint8_t bits, uint8_t bitsz) : m_buf{bits}, m_size{bitsz} {
        assert(m_size > 0 && m_size <= 4);
    }
    bitarray4(bitview bv) : m_buf{*bv.data()}, m_size{(uint8_t)bv.bitsize()} {
        assert(m_size > 0 && m_size <= 4);
    }

    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }
    const uint8_t *data() const {
        return &m_buf;
    }
    varbit::sz_t bitsize() const {
        return m_size;
    }
    varbit::sz_t bytesize() const {
        return sizeof(m_buf);
    }

private:
    uint8_t m_buf;
    uint8_t m_size;
};

class bitarray10 {
public:
    bitarray10(uint16_t bits, uint8_t bitsz) : m_buf{bits}, m_size{bitsz} {
        assert(m_size > 0 && m_size <= 10);
    }
    bitarray10(bitview bv) : m_size{(uint8_t)bv.bitsize()} {
        assert(m_size > 0 && m_size <= 10);
        memcpy(&m_buf, bv.data(), bv.bytesize());
    }

    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }
    const uint8_t *data() const {
        return (uint8_t *)&m_buf;
    }
    varbit::sz_t bitsize() const {
        return m_size;
    }
    varbit::sz_t bytesize() const {
        return sizeof(m_buf);
    }

private:
    uint16_t m_buf;
    uint8_t m_size;
};

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

namespace varbit {

static constexpr sz_t bytesize4bitsize(sz_t bitsz) {
    return roundup_pow2_mul(bitsz, 8) / 8;
}

// bit 0: is_not_ptr
enum class tag_ty : uint8_t {
    ptr          = 0b00,
    ptr_inline56 = 0b10,
    inline4      = 0b01,
    inline10     = 0b11
};

// bit 0: is_not_ptr
enum class raw_buf_ptr_tag_ty : uint8_t {
    ptr     = 0b0,
    inlined = 0b1
};

enum class ptr_ty : uint8_t {
    b10_inline72,
    b10_heap,
    b16_inline_120,
    b16_heap,
};

class SURF_PACKED tag_byte_inline4 {
public:
    SURF_SCA tag_ty_bitpos        = 0;
    SURF_SCA tag_ty_sz            = 2;
    SURF_SCA nbits_minus_1_bitpos = 2;
    SURF_SCA nbits_minus_1_sz     = 2;
    SURF_SCA bits_bitpos          = 4;
    SURF_SCA bits_sz              = 2;
    static_assert(tag_ty_sz + nbits_minus_1_sz + bits_sz <= sizeof(uint8_t) * CHAR_BIT);

    tag_byte_inline4(bitview bv) {
        assert(bv.bitsize() > 0 && bv.bitsize() <= 4);
        m_byte = (magic_enum::enum_integer(tag_ty::inline4) & pow2_mask(tag_ty_sz))
                 << tag_ty_bitpos;
        m_byte |= ((bv.bitsize() - 1) & pow2_mask(nbits_minus_1_sz)) << nbits_minus_1_bitpos;
        m_byte |= (*bv.data() & pow2_mask(bits_sz)) << bits_bitpos;
    }

    uint8_t bitsize() const {
        return 1 + ((m_byte >> nbits_minus_1_bitpos) & pow2_mask(nbits_minus_1_sz));
    }
    uint8_t bytesize() const {
        return bytesize4bitsize(bitsize());
    }

    bitarray4 bitarray4() {
        uint8_t bits = (m_byte >> bits_bitpos) & pow2_mask(bits_sz);
        return surf::bitarray4(bits, bitsize());
    }

private:
    uint8_t m_byte;
};
static_assert(sizeof(tag_byte_inline4) == sizeof(uint8_t));

class SURF_PACKED tag_word_inline10 {
public:
    SURF_SCA tag_ty_bitpos = 0;
    SURF_SCA tag_ty_sz     = 2;
    SURF_SCA nbits_bitpos  = 2;
    SURF_SCA nbits_sz      = 4;
    SURF_SCA bits_bitpos   = 6;
    SURF_SCA bits_sz       = 10;
    static_assert(tag_ty_sz + nbits_sz + bits_sz <= sizeof(uint16_t) * CHAR_BIT);

    tag_word_inline10(bitview bv) {
        assert(bv.bitsize() > 0 && bv.bitsize() <= 10);
        m_word = (magic_enum::enum_integer(tag_ty::inline10) & pow2_mask(tag_ty_sz))
                 << tag_ty_bitpos;
        m_word |= (bv.bitsize() & pow2_mask(nbits_sz)) << nbits_bitpos;
        uint16_t bits;
        memcpy(&bits, bv.data(), bv.bytesize());
        m_word |= (bits & pow2_mask(bits_sz)) << bits_bitpos;
    }

    uint8_t bitsize() const {
        return ((m_word >> nbits_bitpos) & pow2_mask(nbits_sz));
    }
    uint8_t bytesize() const {
        return bytesize4bitsize(bitsize());
    }

    bitarray10 bitarray10() {
        uint16_t bits = (m_word >> bits_bitpos) & pow2_mask(bits_sz);
        return surf::bitarray10(bits, bitsize());
    }

private:
    uint16_t m_word;
};
static_assert(sizeof(tag_word_inline10) == sizeof(uint16_t));

class SURF_PACKED tagged_ptr_inline56 {
public:
    SURF_SCA tag_ty_bitpos = 0;
    SURF_SCA tag_ty_sz     = 2;
    SURF_SCA nbits_bitpos  = 2;
    SURF_SCA nbits_sz      = 6;
    static_assert(tag_ty_sz + nbits_sz <= sizeof(uint8_t) * CHAR_BIT);

    tagged_ptr_inline56(bitview bv) {
        assert(bv.bitsize() > 0 && bv.bitsize() <= 56);
        m_tag_byte = (magic_enum::enum_integer(tag_ty::ptr_inline56) & pow2_mask(tag_ty_sz))
                     << tag_ty_bitpos;
        m_tag_byte |= (bv.bitsize() & pow2_mask(nbits_sz)) << nbits_bitpos;
        memcpy(m_buf, bv.data(), bv.bytesize());
    }

    const uint8_t *data() const {
        return m_buf;
    }
    uint8_t bitsize() const {
        return ((m_tag_byte >> nbits_bitpos) & pow2_mask(nbits_sz));
    }
    uint8_t bytesize() const {
        return bytesize4bitsize(bitsize());
    }

    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }

private:
    uint8_t m_tag_byte;
    uint8_t m_buf[7];
};
static_assert(sizeof(tagged_ptr_inline56) == sizeof(void *));

union tagged_ptr {
    const uintptr_t m_ptrint;
    tagged_ptr_inline56 m_inline56;
};

class SURF_PACKED heap_array_b10_inline72 {
public:
    SURF_SCA raw_buf_ptr_tag_ty_bitpos = 0;
    SURF_SCA raw_buf_ptr_tag_ty_sz     = 1;
    SURF_SCA nbits_bitpos              = 1;
    SURF_SCA nbits_sz                  = 7;
    static_assert(raw_buf_ptr_tag_ty_bitpos + nbits_sz <= sizeof(uint8_t) * CHAR_BIT);

    heap_array_b10_inline72(bitview bv) {
        assert(bv.bitsize() > 0 && bv.bitsize() <= 72);
        m_buf_tag = (magic_enum::enum_integer(raw_buf_ptr_tag_ty::inlined) &
                     pow2_mask(raw_buf_ptr_tag_ty_sz))
                    << raw_buf_ptr_tag_ty_bitpos;
        m_buf_tag |= (bv.bitsize() & pow2_mask(nbits_sz)) << nbits_bitpos;
        memcpy(m_buf, bv.data(), bv.bytesize());
    }

    const uint8_t *data() const {
        return m_buf;
    }
    uint8_t bitsize() const {
        return ((m_buf_tag >> nbits_bitpos) & pow2_mask(nbits_sz));
    }
    uint8_t bytesize() const {
        return bytesize4bitsize(bitsize());
    }

    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }

private:
    uint8_t m_buf_tag;
    uint8_t m_buf[9];
};
static_assert(sizeof(heap_array_b10_inline72) == 10);

struct SURF_PACKED heap_array_b10_ptr {
    std::unique_ptr<uint8_t[]> m_buf;
    sz_t m_size;
};

union heap_array_b10 {
    uint8_t m_tag_byte;
    heap_array_b10_inline72 m_inline72;
    heap_array_b10_ptr m_buf_ptr;
};

class SURF_PACKED heap_array_b16_inline120 {
public:
    SURF_SCA raw_buf_ptr_tag_ty_bitpos = 0;
    SURF_SCA raw_buf_ptr_tag_ty_sz     = 1;
    SURF_SCA nbits_bitpos              = 1;
    SURF_SCA nbits_sz                  = 7;
    static_assert(raw_buf_ptr_tag_ty_bitpos + nbits_sz <= sizeof(uint8_t) * CHAR_BIT);

    heap_array_b16_inline120(bitview bv) {
        assert(bv.bitsize() > 0 && bv.bitsize() <= 120);
        m_buf_tag = (magic_enum::enum_integer(raw_buf_ptr_tag_ty::inlined) &
                     pow2_mask(raw_buf_ptr_tag_ty_sz))
                    << raw_buf_ptr_tag_ty_bitpos;
        m_buf_tag |= (bv.bitsize() & pow2_mask(nbits_sz)) << nbits_bitpos;
        memcpy(m_buf, bv.data(), bv.bytesize());
    }

    const uint8_t *data() const {
        return m_buf;
    }
    uint8_t bitsize() const {
        return ((m_buf_tag >> nbits_bitpos) & pow2_mask(nbits_sz));
    }
    uint8_t bytesize() const {
        return bytesize4bitsize(bitsize());
    }

    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }

private:
    uint8_t m_buf_tag;
    uint8_t m_buf[15];
};
static_assert(sizeof(heap_array_b16_inline120) == 16);

struct SURF_PACKED heap_array_b16_ptr {
    std::unique_ptr<uint8_t[]> m_buf;
    sz_t m_size;
};

// bit 0: is_ptr
// bit 1: is_bit
// if (!is_ptr && !is_bit)
// bit 2-7: size (0 - 56 bytes)
// else
// bit 2: bitval
class tag_byte {
public:
    SURF_SCA is_inlined_bitpos = 0;
    SURF_SCA is_inlined_sz     = 1;
    SURF_SCA is_bit_bitpos     = 1;
    SURF_SCA is_bit_sz         = 1;
    SURF_SCA size_bitpos       = 2;
    SURF_SCA size_sz           = 6;
    SURF_SCA bitval_bitpos     = 2;
    SURF_SCA bitval_sz         = 1;
    static_assert(is_inlined_bitpos + is_bit_sz + size_sz <= sizeof(uint8_t) * CHAR_BIT);

    uint8_t bitsize() const {
        return (m_byte >> size_bitpos) & pow2_mask(size_sz);
    }
    bool is_ptr() const {
        return !((m_byte >> is_inlined_bitpos) & pow2_mask(is_inlined_sz));
    }
    bool is_bit() const {
        return (m_byte >> is_bit_bitpos) & pow2_mask(is_bit_sz);
    }
    SURF_SCA for_inlined(sz_t bitsz) {
        return tag_byte{bitsz};
    }
    SURF_SCA for_ptr() {
        return tag_byte{};
    }
    SURF_SCA for_bit(bool bit) {
        return tag_byte{bit};
    }

private:
    constexpr tag_byte() : m_byte{} {}

    constexpr tag_byte(sz_t bitsz) {
        assert(bitsz <= 56);
        assert(bitsz < pow2(size_sz));
        m_byte |= pow2(is_inlined_bitpos);
        m_byte |= bitsz << size_bitpos;
    }

    constexpr tag_byte(bool bit) {
        m_byte |= pow2(is_inlined_bitpos);
        m_byte |= pow2(is_bit_bitpos);
        m_byte |= ((uint8_t)bit) << bitval_bitpos;
    }

    uint8_t m_byte;
};
static_assert(sizeof(tag_byte) == 1);

// FIXME: little endian only right now
class varbit_inline {
public:
    constexpr static sz_t bytesize(sz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    varbit_inline() : m_tag{tag_byte::for_ptr()}, m_buf{} {}
    varbit_inline(bitview bv) : m_tag{tag_byte::for_inlined(bv.bitsize())} {
        std::copy(bv.data(), bv.data() + bv.bytesize(), m_buf);
    }
    varbit_inline(bool bit) : m_tag{tag_byte::for_bit(bit)} {}

    const uint8_t *data() const {
        return m_buf;
    }
    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }
    sz_t bitsize() const {
        return m_tag.bitsize();
    }
    sz_t bytesize() const {
        return bytesize(bitsize());
    }
    tag_byte tag() const {
        return m_tag;
    }

private:
    tag_byte m_tag;
    uint8_t m_buf[7];
};
static_assert(sizeof(varbit_inline) == sizeof(void *));

class varbit_heap {
public:
    constexpr static sz_t bytesize(sz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    varbit_heap(bitview bv) {
        m_buf  = std::unique_ptr<uint8_t[]>{new uint8_t[bv.bytesize()]};
        m_size = bv.bitsize();
        std::copy(bv.data(), bv.data() + bv.bytesize(), m_buf.get());
    }
    const uint8_t *data() const {
        return m_buf.get();
    }
    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }
    sz_t bitsize() const {
        return m_size;
    }
    sz_t bytesize() const {
        return bytesize(bitsize());
    }

private:
    std::unique_ptr<uint8_t[]> m_buf;
    sz_t m_size;
};
static_assert(sizeof(varbit_heap) <= 16);
static_assert(alignof(varbit_heap) >= alignof(void *));

}; // namespace varbit

union SURF_EXPORT VarBit {
public:
    constexpr static varbit::sz_t bytesize(varbit::sz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    VarBit(bitview bv) {
        if (SURF_LIKELY(bv.bitsize() <= 7 * CHAR_BIT)) {
            if (bv.bitsize() == 1) {
                m_inlined = varbit::varbit_inline{(bool)(*bv.data() & 1)};
            } else {
                m_inlined = varbit::varbit_inline{bv};
            }
        } else {
            m_heap = new varbit::varbit_heap{bv};
        }
    }
    ~VarBit() {
        if (SURF_UNLIKELY(m_inlined.tag().is_ptr())) {
            delete m_heap;
        }
    }

    const uint8_t *data() const {
        if (SURF_LIKELY(!m_inlined.tag().is_ptr())) {
            return m_inlined.data();
        }
        return m_heap->data();
    }
    bitview bitview() const {
        return surf::bitview{data(), bitsize()};
    }
    varbit::sz_t bitsize() const {
        if (SURF_LIKELY(!m_inlined.tag().is_ptr())) {
            return m_inlined.bitsize();
        }
        return m_heap->bitsize();
    }
    varbit::sz_t bytesize() const {
        return bytesize(bitsize());
    }

private:
    varbit::varbit_heap *m_heap;
    varbit::varbit_inline m_inlined;
};
static_assert(sizeof(VarBit) == sizeof(void *));

}; // namespace surf

template <> struct fmt::formatter<surf::bitview> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::bitview const &bv, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "<bitview[{:d}] {:s}>", bv.bitsize(),
                              surf::bitview2string(bv));
    }
};

template <> struct fmt::formatter<surf::VarBit> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VarBit const &vb, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "<VarBit[{:d}] {:s}>", vb.bitsize(),
                              surf::bitview2string(vb.bitview()));
    }
};
