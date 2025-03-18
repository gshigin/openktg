#pragma once

#define OKTG(x, ...) OKTG_IMPL_##x(__VA_ARGS__)

#if defined(_MSC_VER)
#define OKTG_IMPL_always_inline() __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define OKTG_IMPL_always_inline() __attribute__((always_inline)) inline
#else
#define OKTG_IMPL_always_inline() inline
#endif