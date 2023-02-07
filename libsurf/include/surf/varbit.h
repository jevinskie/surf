#pragma once

#include "common.h"

namespace surf {

class SURF_EXPORT VarBit {
public:
private:
};

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