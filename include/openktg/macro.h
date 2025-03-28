#pragma once

#define OKTG(x, ...) OKTG_IMPL_##x(__VA_ARGS__)

// compiler detection
#ifdef _MSC_VER
#define OKTG_IMPL_compiler_msvc() 1
#define OKTG_IMPL_compiler_gcc() 0
#define OKTG_IMPL_compiler_clang() 0
#elif defined(__clang__)
#define OKTG_IMPL_compiler_msvc() 0
#define OKTG_IMPL_compiler_gcc() 0
#define OKTG_IMPL_compiler_clang() 1
#elif defined(__GNUC__) & !defined(__clang__)
#define OKTG_IMPL_compiler_msvc() 0
#define OKTG_IMPL_compiler_gcc() 1
#define OKTG_IMPL_compiler_clang() 0
#else
#error Unsopported compiler!
#endif

// helper macro for compiler checks
#define OKTG_IMPL_compiler(x) OKTG_IMPL_compiler_##x()

// inline
#if OKTG(compiler, msvc)
#define OKTG_IMPL_always_inline() __forceinline
#elif OKTG(compiler, clang) || OKTG(compiler, gcc)
#define OKTG_IMPL_always_inline() __attribute__((always_inline)) inline
#else
#define OKTG_IMPL_always_inline() inline
#endif