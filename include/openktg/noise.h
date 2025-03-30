#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>

#include <openktg/random.h>
#include <openktg/utility.h>

struct PerlinNoise
{
    static constexpr std::size_t TableSize = 4096;
    static constexpr std::size_t seed = 0x93638245u;

    static constexpr auto GeneratePtable(std::uint32_t seed) -> std::array<std::uint16_t, TableSize>
    {
        std::array<std::uint16_t, TableSize> table{};
        std::iota(table.begin(), table.end(), 0);

        openktg::random::xoshiro128ss rng{openktg::random::seed(seed), openktg::random::seed(openktg::random::seed(seed))};
        for (std::size_t i = TableSize - 1; i > 0; --i)
        {
            std::swap(table[i], table[rng() % (i + 1)]);
        }

        return table;
    }

    inline static std::array<std::uint16_t, TableSize> Ptable = GeneratePtable(seed);

    [[nodiscard]] static constexpr auto P(int i) -> int
    {
        return Ptable[i & (TableSize - 1)];
    }

    [[nodiscard]] static constexpr auto PGradient2(int hash, float x, float y) -> float
    {
        constexpr float signs[8][2] = {{1.0f, 2.0f}, {-1.0f, 2.0f}, {1.0f, -2.0f}, {-1.0f, -2.0f}, {1.0f, 2.0f}, {-1.0f, 2.0f}, {1.0f, -2.0f}, {-1.0f, -2.0f}};

        hash &= 7;
        return x * signs[hash][0] + y * signs[hash][1];
    }

    [[nodiscard]] static constexpr auto Noise2(int x, int y, int maskx, int masky, int seed) -> float
    {
        constexpr float InvM = 1.0f / 0x10000;
        const int X = x >> 16, Y = y >> 16;
        const float fx = (x & 0xFFFF) * InvM;
        const float fy = (y & 0xFFFF) * InvM;

        const float u = SmoothStep(fx);
        const float v = SmoothStep(fy);

        maskx &= TableSize - 1;
        masky &= TableSize - 1;

        const int px0 = X & maskx;
        const int px1 = (X + 1) & maskx;
        const int py0 = Y & masky;
        const int py1 = (Y + 1) & masky;

        const int Ppy0 = P(py0);
        const int Ppy1 = P(py1);

        const float p00 = (P(px0 + Ppy0 + seed) / 2047.5f) - 1.0f;
        const float p10 = (P(px1 + Ppy0 + seed) / 2047.5f) - 1.0f;
        const float p01 = (P(px0 + Ppy1 + seed) / 2047.5f) - 1.0f;
        const float p11 = (P(px1 + Ppy1 + seed) / 2047.5f) - 1.0f;

        return openktg::utility::lerp(openktg::utility::lerp(p00, p10, u), openktg::utility::lerp(p01, p11, u), v);
    }

    static constexpr auto SmoothStep(float x) -> float
    {
        return x * x * x * (10 + x * (6 * x - 15));
    }

    static constexpr auto GShuffle(int x, int y, int z) -> int
    {
        return P(P(P(x) + y) + z);
    }

    // 2D grid noise function (tiling)
    static constexpr auto GNoise2(int x, int y, int maskx, int masky, int seed) -> float
    {
        int i = x >> 16;
        int j = y >> 16;
        float xp = (x & 0xFFFF) * (1.0f / 65536.0f);
        float yp = (y & 0xFFFF) * (1.0f / 65536.0f);
        float sum = 0.0f;

        for (int oy = 0; oy <= 1; oy++)
        {
            for (int ox = 0; ox <= 1; ox++)
            {
                float xr = xp - ox;
                float yr = yp - oy;
                float t = 1.0f - (xr * xr + yr * yr);

                if (t > 0.0f)
                {
                    t = t * t * t * t; // pow(t, 4)
                    sum += t * PGradient2(GShuffle((i + ox) & maskx, (j + oy) & masky, seed), xr, yr);
                }
            }
        }

        return sum;
    }
};