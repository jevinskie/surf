#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
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
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <hedley.h>
#include <magic_enum/magic_enum.hpp>

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

#if defined(__GNUC__) || defined(__clang__)
#define SURF_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define SURF_PACKED __declspec(align(1))
#else
#warning "Compiler does not support packed attribute."
#define SURF_PACKED
#endif

#define SURF_SCA static constexpr auto

#define SURF_EMPTY_SYM "∅"
#define SURF_TRUE_SYM "✅"
#define SURF_FALSE_SYM "❌"

#if defined(__aarch64__) && __has_include(<arm_neon.h>)
#define SURF_A64_NEON
#endif

namespace surf {

constexpr auto pow2(uint8_t n) {
    return 1 << n;
}

constexpr auto pow2_mask(uint8_t n) {
    return pow2(n) - 1;
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
// rounddown_pow2_mul(16, 16) = 16
// rounddown_pow2_mul(17, 16) = 16
template <typename U>
    requires requires() { requires std::unsigned_integral<U>; }
constexpr U rounddown_pow2_mul(U num, size_t pow2_mul) {
    const U mask = static_cast<U>(pow2_mul) - 1;
    return num & ~mask;
}

template <typename T> consteval size_t sizeofbits() {
    return sizeof(T) * CHAR_BIT;
}

consteval size_t sizeofbits(const auto &o) {
    return sizeof(o) * CHAR_BIT;
}

template <typename T> size_t bytesizeof(const typename std::vector<T> &vec) {
    return sizeof(T) * vec.size();
}

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
