#pragma once

#include "common-internal.h"

#include <bit>

#if 0
#include <backward.hpp>

#define DUMP_STACK(name)                                                                           \
    do {                                                                                           \
        {                                                                                          \
            std::cout << (name) << " STACK BEGIN:" << std::endl;                                   \
            backward::StackTrace st;                                                               \
            st.load_here(32);                                                                      \
            backward::Printer pr;                                                                  \
            pr.snippet    = true;                                                                  \
            pr.color_mode = backward::ColorMode::always;                                           \
            pr.address    = true;                                                                  \
            pr.object     = true;                                                                  \
            pr.print(st);                                                                          \
            std::cout << (name) << " STACK END" << std::endl;                                      \
        }                                                                                          \
    } while (0)
#define MCA_BEGIN(name)                                                                            \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-BEGIN " #name ::: "memory");                                    \
    } while (0)
#define MCA_END()                                                                                  \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-END" ::: "memory");                                             \
    } while (0)
#endif

namespace surf {

template <typename T> size_t bytesizeof(const typename std::vector<T> &vec) {
    return sizeof(T) * vec.size();
}

template <typename T> constexpr bool is_pow2(T num) {
    return std::popcount(num) == 1;
}

// behavior:
// roundup_pow2_mul(16, 16) = 16
// roundup_pow2_mul(17, 16) = 32
template <typename U>
    requires requires() { requires std::unsigned_integral<U>; }
constexpr U roundup_pow2_mul(U num, size_t pow2_mul) {
    const U mask = static_cast<U>(pow2_mul) - 1;
    return (num + mask) & ~mask;
}

// behavior:
// roundup_pow2_mul(16, 16) = 16
// roundup_pow2_mul(17, 16) = 16
template <typename U>
    requires requires() { requires std::unsigned_integral<U>; }
constexpr U rounddown_pow2_mul(U num, size_t pow2_mul) {
    const U mask = static_cast<U>(pow2_mul) - 1;
    return num & ~mask;
}

void posix_check(int retval, const std::string &msg);

unsigned int get_num_cores();

bool can_use_term_colors();

template <typename T> constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name   = __PRETTY_FUNCTION__;
    prefix = "auto surf::type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name   = __PRETTY_FUNCTION__;
    prefix = "constexpr auto surf::type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name   = __FUNCSIG__;
    prefix = "auto __cdecl surf::type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

// for visit(overload(...case lambdas...), variant_var)
template <typename... T> class overload : T... {
public:
    overload(T... t) : T(t)... {}
    using T::operator()...;
};

// https://www.modernescpp.com/index.php/c-20-pythons-map-function
template <typename Func, typename Seq> auto map(Func func, Seq seq) {
    typedef typename Seq::value_type value_type;
    using return_type = decltype(func(std::declval<value_type>()));

    std::vector<return_type> result{};
    for (auto &i : seq) {
        result.emplace_back(func(i));
    }

    return result;
}

}; // namespace surf

// Credit: Arthaud Awen / davawen
// https://github.com/fmtlib/fmt/issues/1367#issuecomment-1229916316
template <typename T> struct fmt::formatter<std::optional<T>> {
    std::string_view underlying_fmt;
    std::string_view or_else;

    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        // {:<if present{}><if not present>}
        auto it = ctx.begin(), end = ctx.end();
        auto get_marker = [&it, end]() constexpr {
            if (it == end || *it != '<')
                return std::string_view(nullptr, 0); // no match
            auto start = ++it;

            while (it != end && (*it++ != '>'))
                ;
            if (it == end)
                throw fmt::format_error("invalid format, unfinished range");

            return std::string_view(start, (it - 1) - start);
        };

        underlying_fmt = "{}";
        or_else        = SURF_EMPTY_SYM;

        auto first = get_marker();
        if (first.data())
            underlying_fmt = first;

        auto second = get_marker();
        if (second.data())
            or_else = second;

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw fmt::format_error("invalid format, no end bracket");
        return it;
    }

    template <typename FormatContext>
    auto format(const std::optional<T> &p, FormatContext &ctx) const -> decltype(ctx.out()) {
        if (p) {
            return vformat_to(ctx.out(), underlying_fmt, format_arg_store<FormatContext, T>{*p});
        } else {
            return format_to(ctx.out(), "{}", or_else);
        }
    }
};