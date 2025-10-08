#pragma once

#include "common.h"

namespace surf {

class SURF_EXPORT Time {
public:
    Time();
    Time(uint64_t ticks, int timebase_power);
    Time(double seconds, int timebase_power);
    uint64_t ticks() const;
    double seconds() const;
    int timebase_power() const;
    static uint64_t seconds_to_ticks(double seconds, int timebase_power);
    static double ticks_to_seconds(uint64_t seconds, int timebase_power);

private:
    int m_timebase_power;
    uint64_t m_ticks;
};

} // namespace surf

template <> struct fmt::formatter<surf::Time> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::Time const &time, FormatContext &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "<Time tbp: {} ticks: {}>", time.timebase_power(),
                              time.ticks());
    }
};
