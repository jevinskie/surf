#pragma once

#include "common.h"
#include "time.h"

namespace surf {

class SURF_EXPORT Trace {
public:
    Trace(const std::filesystem::path &path);
    int timebase_power() const;
    Time start() const;
    Time end() const;

private:
    const std::filesystem::path m_path;
    int m_fd;
    int m_timebase_power;
    Time m_start;
    Time m_end;
};

} // namespace surf

template <> struct fmt::formatter<surf::Trace> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::Trace const &trace, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "<Trace tbp: {} start: {} end: {}>",
                              trace.timebase_power(), trace.start(), trace.end());
    }
};
