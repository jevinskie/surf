#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#define SURF_EXPORT __attribute__((visibility("default")))
#define SURF_INLINE __attribute__((always_inline))
// #define SURF_INLINE
#define SURF_NOINLINE __attribute__((noinline))
#define SURF_LIKELY(cond) __builtin_expect((cond), 1)
#define SURF_UNLIKELY(cond) __builtin_expect((cond), 0)
#define SURF_BREAK() __builtin_debugtrap()
#define SURF_ALIGNED(n) __attribute__((aligned(n)))
#define SURF_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), n)
#define SURF_UNREACHABLE() __builtin_unreachable()

namespace surf {};