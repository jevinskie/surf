#pragma once

#include "common-internal.h"

#include <bit>

#define MCA_BEGIN(name)                                                                            \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-BEGIN " #name ::: "memory");                                    \
    } while (0)
#define MCA_END()                                                                                  \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-END" ::: "memory");                                             \
    } while (0)

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

SURF_EXPORT void posix_check(int retval, const std::string &msg);

SURF_EXPORT unsigned int get_num_cores();
