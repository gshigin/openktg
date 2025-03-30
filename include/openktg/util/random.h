#pragma once

#include <cstdint>

namespace openktg::random
{

namespace _random
{
constexpr auto rotl(uint64_t x, int k) noexcept -> std::uint64_t
{
    return (x << k) | (x >> (64 - k));
}
} // namespace _random

constexpr auto seed(std::uint64_t seed_) -> std::uint64_t
{
    constexpr std::uint64_t C1 = 0x9e3779b97f4a7c15;
    constexpr std::uint64_t C2 = 0xbf58476d1ce4e5b9;
    constexpr std::uint64_t C3 = 0x94d049bb133111eb;

    seed_ += C1;
    seed_ ^= seed_ >> 30;
    seed_ *= C2;
    seed_ ^= seed_ >> 27;
    seed_ *= C3;
    seed_ ^= seed_ >> 31;
    return seed_;
}

// uses time of compilation for seeding rng
constexpr auto seed_time() -> std::uint64_t
{
    return seed(static_cast<std::uint64_t>(__TIME__[0]) << 56 | static_cast<std::uint64_t>(__TIME__[1]) << 48 | static_cast<std::uint64_t>(__TIME__[3]) << 40 |
                static_cast<std::uint64_t>(__TIME__[4]) << 32 | static_cast<std::uint64_t>(__TIME__[6]) << 24 | static_cast<std::uint64_t>(__TIME__[7]) << 16);
}

struct xoshiro128ss
{
    std::uint64_t state[2];

    constexpr auto next() noexcept -> std::uint64_t
    {
        const std::uint64_t s0 = state[0];
        std::uint64_t s1 = state[1];
        std::uint64_t result = _random::rotl(s0 * 5, 7) * 9;

        s1 ^= s0;
        state[0] = _random::rotl(s0, 24) ^ s1 ^ (s1 << 16);
        state[1] = _random::rotl(s1, 37);

        return result;
    }

    constexpr auto operator()() noexcept -> std::uint64_t
    {
        return next();
    }

    constexpr auto fork() noexcept -> xoshiro128ss
    {
        return xoshiro128ss{next(), next()};
    }

    static constexpr auto min() noexcept -> std::uint64_t
    {
        return 0;
    }
    static constexpr auto max() noexcept -> std::uint64_t
    {
        return UINT64_MAX;
    }
    using result_type = std::uint64_t;
};

struct xoshiro128pp
{
    std::uint64_t state[2];

    constexpr auto next() noexcept -> std::uint64_t
    {
        const std::uint64_t s0 = state[0];
        std::uint64_t s1 = state[1];
        std::uint64_t result = _random::rotl(s0 + s1, 17) + s0;

        s1 ^= s0;
        state[0] = _random::rotl(s0, 49) ^ s1 ^ (s1 << 21);
        state[1] = _random::rotl(s1, 28);

        return result;
    }

    constexpr auto operator()() noexcept -> std::uint64_t
    {
        return next();
    }

    constexpr auto fork() noexcept -> xoshiro128ss
    {
        return xoshiro128ss{next(), next()};
    }

    static constexpr auto min() noexcept -> std::uint64_t
    {
        return 0;
    }
    static constexpr auto max() noexcept -> std::uint64_t
    {
        return UINT64_MAX;
    }
    using result_type = std::uint64_t;
};

struct xoshiro256ss
{
    uint64_t state[4];

    constexpr auto next() noexcept -> std::uint64_t
    {
        const std::uint64_t result = _random::rotl(state[0] + state[3], 23) + state[0];

        const std::uint64_t t = state[1] << 17;

        state[2] ^= state[0];
        state[3] ^= state[1];
        state[1] ^= state[2];
        state[0] ^= state[3];

        state[2] ^= t;

        state[3] = _random::rotl(state[3], 45);

        return result;
    }

    constexpr auto operator()() noexcept -> std::uint64_t
    {
        return next();
    }

    constexpr auto fork() noexcept -> xoshiro256ss
    {
        return xoshiro256ss{next(), next(), next(), next()};
    }

    static constexpr auto min() noexcept -> std::uint64_t
    {
        return 0;
    }
    static constexpr auto max() noexcept -> std::uint64_t
    {
        return UINT64_MAX;
    }
    using result_type = std::uint64_t;
};

struct xoshiro256pp
{
    uint64_t state[4];

    constexpr auto next() noexcept -> std::uint64_t
    {
        const std::uint64_t result = _random::rotl(state[1] * 5, 7) * 9;

        const std::uint64_t t = state[1] << 17;

        state[2] ^= state[0];
        state[3] ^= state[1];
        state[1] ^= state[2];
        state[0] ^= state[3];

        state[2] ^= t;

        state[3] = _random::rotl(state[3], 45);

        return result;
    }

    constexpr auto operator()() noexcept -> std::uint64_t
    {
        return next();
    }

    constexpr auto fork() noexcept -> xoshiro256ss
    {
        return xoshiro256ss{next(), next(), next(), next()};
    }

    static constexpr auto min() noexcept -> std::uint64_t
    {
        return 0;
    }
    static constexpr auto max() noexcept -> std::uint64_t
    {
        return UINT64_MAX;
    }
    using result_type = std::uint64_t;
};
} // namespace openktg::random