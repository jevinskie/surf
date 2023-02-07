#pragma once

#include "common.h"

#include <array>

#include <mz/tagged_ptr.hpp>

namespace surf {

namespace varbit {

class tag_byte {
public:
    tag_byte(uint8_t size, bool is_signed) {
        assert(size <= 7);
        m_byte = (is_signed << 3) | (size & 0b111);
    }
    uint8_t size() const {
        return m_byte & 0b111;
    }
    bool is_signed() const {
        return m_byte >> 3;
    }

private:
    uint8_t m_byte;
};
static_assert(sizeof(tag_byte) == 1);

// FIXME: little endian only right now
struct varbit_inline {
    tag_byte tag;
    std::array<uint8_t, 7> buf;
};
static_assert(sizeof(varbit_inline) == sizeof(void *));

union tagged_union {
    mz::tagged_ptr<std::vector<uint8_t>> heap;
    varbit_inline inlined;
};
static_assert(sizeof(tagged_union) == sizeof(void *));

}; // namespace varbit

class SURF_EXPORT VarBit {
public:
private:
    varbit::tagged_union m_storage;
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
