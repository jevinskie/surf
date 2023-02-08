#pragma once

#include "common.h"

namespace surf {

namespace varbit {

using sz_t  = int16_t;
using usz_t = std::make_unsigned_t<sz_t>;

// bit 0: is_ptr
// bit 1-6: size (0 - 56 bytes)
// bit 7: is_signed
class tag_byte {
public:
    static constexpr auto is_ptr_bitpos    = 0;
    static constexpr auto is_ptr_sz        = 1;
    static constexpr auto size_bitpos      = 1;
    static constexpr auto size_sz          = 6;
    static constexpr auto is_signed_bitpos = 7;
    static constexpr auto is_signed_sz     = 1;
    static_assert(is_ptr_sz + size_sz + is_signed_sz <= sizeof(uint8_t) * CHAR_BIT);

    uint8_t size() const {
        return (m_byte >> size_bitpos) & pow2_mask(size_sz);
    }
    bool is_signed() const {
        return (m_byte >> is_signed_bitpos) & pow2_mask(is_signed_sz);
    }
    bool is_ptr() const {
        return (m_byte >> is_ptr_bitpos) & pow2_mask(is_ptr_sz);
    }
    static constexpr auto for_inlined(usz_t bitsz, bool is_signed) {
        return tag_byte(bitsz, is_signed);
    }
    static constexpr auto for_ptr() {
        return tag_byte{};
    }

private:
    constexpr tag_byte() : m_byte{} {}

    constexpr tag_byte(uint8_t size, bool is_signed) {
        assert(size <= 56);
        assert(size < pow2(size_sz));
        m_byte = pow2(is_ptr_bitpos);
        m_byte |= is_signed << is_signed_bitpos;
        m_byte |= size << size_bitpos;
    }

    uint8_t m_byte;
};
static_assert(sizeof(tag_byte) == 1);

// FIXME: little endian only right now
class varbit_inline {
public:
    constexpr static usz_t bytesize(usz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    varbit_inline() : m_tag{tag_byte::for_ptr()} {}
    varbit_inline(usz_t bitsz, bool is_signed) : m_tag{tag_byte::for_inlined(bitsz, is_signed)} {}

    const uint8_t *data() const {
        return m_buf;
    }
    usz_t bitsize() const {
        return m_tag.size();
    }
    usz_t bytesize() const {
        return bytesize(bitsize());
    }
    bool is_signed() const {
        return m_tag.is_signed();
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
    constexpr static usz_t bytesize(usz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    varbit_heap(const uint8_t *buf, usz_t bitsz, bool is_signed) {
        m_buf           = std::unique_ptr<uint8_t[]>{new uint8_t[bytesize(bitsz)]};
        m_size_and_sign = bitsz * (-1 * is_signed);
        std::copy(buf, buf + bytesize(bitsz), m_buf.get());
    }
    const uint8_t *data() const {
        return m_buf.get();
    }
    usz_t bitsize() const {
        return std::abs(m_size_and_sign);
    }
    usz_t bytesize() const {
        return bytesize(bitsize());
    }
    bool is_signed() const {
        return m_size_and_sign < 0;
    }

private:
    std::unique_ptr<uint8_t[]> m_buf;
    sz_t m_size_and_sign;
};
static_assert(sizeof(varbit_heap) <= 16);
static_assert(alignof(varbit_heap) >= alignof(void *));

}; // namespace varbit

union SURF_EXPORT VarBit {
public:
    constexpr static varbit::usz_t bytesize(varbit::usz_t bitsz) {
        return roundup_pow2_mul(bitsz, 8) / 8;
    }

    VarBit(const uint8_t *buf, varbit::usz_t bitsz, bool is_signed = false) {}
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
    varbit::usz_t bitsize() const {
        if (SURF_LIKELY(!m_inlined.tag().is_ptr())) {
            return m_inlined.bitsize();
        }
        return m_heap->bitsize();
    }
    varbit::usz_t bytesize() const {
        return bytesize(bitsize());
    }
    bool is_signed() const {
        if (SURF_LIKELY(!m_inlined.tag().is_ptr())) {
            return m_inlined.is_signed();
        }
        return m_heap->is_signed();
    }

private:
    varbit::varbit_heap *m_heap;
    varbit::varbit_inline m_inlined;
};
static_assert(sizeof(VarBit) == sizeof(void *));

}; // namespace surf

template <> struct fmt::formatter<surf::VarBit> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VarBit const &vb, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "<VarBit>");
    }
};
