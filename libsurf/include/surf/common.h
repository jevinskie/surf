#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <hedley.h>

#ifdef __linux__
#define SURF_LINUX
#endif
#ifdef _WIN32
#define SURF_WIN
#endif
#ifdef __APPLE__
#define SURF_APPLE
#endif

#define SURF_EXPORT __attribute__((visibility("default")))
#define SURF_INLINE __attribute__((always_inline))
// #define SURF_INLINE
#define SURF_NOINLINE __attribute__((noinline))
#define SURF_LIKELY(cond) __builtin_expect(!!(cond), 1)
#define SURF_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#define SURF_BREAK() __builtin_debugtrap()
#define SURF_ALIGNED(n) __attribute__((aligned(n)))
#define SURF_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#define SURF_UNREACHABLE() __builtin_unreachable()
#define SURF_EMPTY_SYM "∅"
#define SURF_TRUE_SYM "✅"
#define SURF_FALSE_SYM "❌"

#if defined(__aarch64__) && __has_include(<arm_neon.h>)
#define SURF_A64_NEON
#endif

namespace surf {

static inline std::string boolmoji(bool b) {
    return b ? SURF_TRUE_SYM : SURF_FALSE_SYM;
}

template <typename T> static constexpr auto type_name() {
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

}; // namespace surf
