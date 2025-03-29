#pragma once

#include <array>
#include <cstdint>

#include <openktg/helpers.h>
#include <openktg/types.h>

struct PerlinNoise
{
    static constexpr size_t TableSize = 4096;
    static constexpr size_t seed = 0x93638245u;

    static constexpr auto GeneratePtable(std::uint32_t seed) -> std::array<sU16, TableSize>
    {
        std::uint32_t poly = 0xc0000401u;
        std::array<sU16, TableSize> table{};

        struct mpair
        {
            sU32 hex;
            sU16 idx;
        };

        std::array<mpair, TableSize> temp{};

        for (std::size_t i = 0; i < TableSize; ++i)
        {
            temp[i] = {seed, static_cast<sU16>(i)};
            seed = (seed << 1) ^ ((seed & 0x80000000u) ? 0xc0000401u : 0);
        }

        std::sort(temp.begin(), temp.end(), [](const auto &a, const auto &b) { return a.hex < b.hex; });

        for (std::size_t i = 0; i < TableSize; ++i)
        {
            table[i] = temp[i].idx;
        }

        return table;
    }

    inline static std::array<sU16, TableSize> Ptable = GeneratePtable(seed);

    // static void initializeTable()
    // {
    //     Ptable = GeneratePtable(seed);
    // }

    [[nodiscard]] static constexpr auto P(sInt i) -> sInt
    {
        return Ptable[i & (TableSize - 1)];
    }

    [[nodiscard]] static constexpr auto PGradient2(sInt hash, sF32 x, sF32 y) -> sF32
    {
        hash &= 7;
        sF32 u = x * ((hash & 4) ? -1.0f : 1.0f);
        sF32 v = y * ((hash & 2) ? -2.0f : 2.0f);
        return ((hash & 1) ? -u : u) + v;
    }

    [[nodiscard]] static constexpr auto Noise2(sInt x, sInt y, sInt maskx, sInt masky, sInt seed) -> sF32
    {
        const sInt M = 0x10000;
        const sInt X = x >> 16, Y = y >> 16;
        const sF32 fx = (x & (M - 1)) * (1.0f / M);
        const sF32 fy = (y & (M - 1)) * (1.0f / M);
        const sF32 u = SmoothStep(fx);
        const sF32 v = SmoothStep(fy);
        maskx &= TableSize - 1;
        masky &= TableSize - 1;

        const sInt px0 = (X + 0) & maskx;
        const sInt px1 = (X + 1) & maskx;
        const sInt py0 = (Y + 0) & masky;
        const sInt py1 = (Y + 1) & masky;

        const sInt Ppy0 = P(py0);
        const sInt Ppy1 = P(py1);

        const sF32 p00 = (P(px0 + Ppy0 + seed) / 2047.5f) - 1.0f;
        const sF32 p10 = (P(px1 + Ppy0 + seed) / 2047.5f) - 1.0f;
        const sF32 p01 = (P(px0 + Ppy1 + seed) / 2047.5f) - 1.0f;
        const sF32 p11 = (P(px1 + Ppy1 + seed) / 2047.5f) - 1.0f;

        return LerpF(v, LerpF(u, p00, p10), LerpF(u, p01, p11));
    }

    static constexpr auto SmoothStep(sF32 x) -> sF32
    {
        return x * x * x * (10 + x * (6 * x - 15));
    }

    static constexpr auto GShuffle(sInt x, sInt y, sInt z) -> sInt
    {
        /*sU32 seed = ((x & 0x3ff) << 20) | ((y & 0x3ff) << 10) | (z & 0x3ff);

        seed ^= seed << 3;
        seed += seed >> 5;
        seed ^= seed << 4;
        seed += seed >> 17;
        seed ^= seed << 25;
        seed += seed >> 6;

        return seed;*/

        return P(P(P(x) + y) + z);
    }

    // 2D grid noise function (tiling)
    static constexpr auto GNoise2(sInt x, sInt y, sInt maskx, sInt masky, sInt seed) -> sF32
    {
        // input coordinates
        sInt i = x >> 16;
        sInt j = y >> 16;
        sF32 xp = (x & 0xffff) / 65536.0f;
        sF32 yp = (y & 0xffff) / 65536.0f;
        sF32 sum = 0.0f;

        // sum over grid vertices
        for (sInt oy = 0; oy <= 1; oy++)
        {
            for (sInt ox = 0; ox <= 1; ox++)
            {
                sF32 xr = xp - ox;
                sF32 yr = yp - oy;

                sF32 t = xr * xr + yr * yr;
                if (t < 1.0f)
                {
                    t = 1.0f - t;
                    t *= t;
                    t *= t;
                    sum += t * PGradient2(GShuffle((i + ox) & maskx, (j + oy) & masky, seed), xr, yr);
                }
            }
        }

        return sum;
    }
};