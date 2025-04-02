#include "openktg/core/matrix.h"
#include <benchmark/benchmark.h>

#include <openktg/core/pixel.h>
#include <openktg/core/types.h>
#include <openktg/legacy/gentexture.h>
#include <openktg/tex/procedural.h>

static void BM_Demo(benchmark::State &state)
{
    using namespace openktg;
    for (auto _ : state)
    {
        // colors
        openktg::pixel black{0xFF000000_argb};
        openktg::pixel white{0xFFFFFFFF_argb};

        // create gradients
        GenTexture gradBW = LinearGradient(0xff000000, 0xffffffff);
        GenTexture gradWB = LinearGradient(0xffffffff, 0xff000000);
        GenTexture gradWhite = LinearGradient(0xffffffff, 0xffffffff);

        // simple noise test texture
        GenTexture noise;
        noise.Init(256, 256);
        Noise(noise, gradBW, 2, 2, 6, 0.5f, 123, NoiseDirect | NoiseBandlimit | NoiseNormalize);

        // 4 "random voronoi" textures with different minimum distances
        GenTexture voro[4];
        static sInt voroIntens[4] = {37, 42, 37, 37};
        static sInt voroCount[4] = {90, 132, 240, 255};
        static sF32 voroDist[4] = {0.125f, 0.063f, 0.063f, 0.063f};

        for (sInt i = 0; i < 4; i++)
        {
            voro[i].Init(256, 256);
            RandomVoronoi(voro[i], gradWhite, voroIntens[i], voroCount[i], voroDist[i]);
        }

        // linear combination of them
        LinearInput inputs[4];
        for (sInt i = 0; i < 4; i++)
        {
            inputs[i].Tex = &voro[i];
            inputs[i].Weight = 1.5f;
            inputs[i].UShift = 0.0f;
            inputs[i].VShift = 0.0f;
            inputs[i].FilterMode = WrapU | WrapV | FilterNearest;
        }

        GenTexture baseTex;
        baseTex.Init(256, 256);
        LinearCombine(baseTex, black, 0.0f, inputs, 4);

        // blur it
        Blur(baseTex, baseTex, 0.0074f, 0.0074f, 1, WrapU | WrapV);

        // add a noise layer
        GenTexture noiseLayer;
        noiseLayer.Init(256, 256);
        Noise(noiseLayer, LinearGradient(0xff000000, 0xff646464), 4, 4, 5, 0.995f, 3, NoiseDirect | NoiseNormalize | NoiseBandlimit);

        Paste(baseTex, baseTex, noiseLayer, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, CombineAdd, 0);

        // colorize it
        Colorize(baseTex, 0xff747d8e, 0xfff1feff);

        // Create transform matrix for grid pattern
        openktg::matrix44<float> m1 = matrix44<float>::translation(-0.5f, -0.5f, 0.0f);
        openktg::matrix44<float> m2 = matrix44<float>::scale(3.0f * sSQRT2F, 3.0f * sSQRT2F, 1.0f);
        openktg::matrix44<float> m3 = m2 * m1;
        m1 = matrix44<float>::rotation_z(0.125f * sPI2F);
        m2 = m1 * m3;
        m1 = matrix44<float>::translation(0.5f, 0.5f, 0.0f);
        m3 = m1 * m2;

        // Grid pattern GlowRect
        GenTexture rect1, rect1x, rect1n;
        rect1.Init(256, 256);
        LinearCombine(rect1, black, 1.0f, 0, 0); // black background
        GlowRect(rect1, rect1, gradWB, 0.5f, 0.5f, 0.41f, 0.0f, 0.0f, 0.25f, 0.7805f, 0.64f);

        rect1x.Init(256, 256);
        CoordMatrixTransform(rect1x, rect1, m3, WrapU | WrapV | FilterBilinear);

        // Make a normalmap from it
        rect1n.Init(256, 256);
        Derive(rect1n, rect1x, DeriveNormals, 2.5f);

        // Apply as bump map
        GenTexture finalTex;
        openktg::pixel amb{0xff101010_argb};
        openktg::pixel diff{0xffffffff_argb};

        finalTex.Init(256, 256);
        Bump(finalTex, baseTex, rect1n, 0, 0, 0.0f, 0.0f, 0.0f, -2.518f, 0.719f, -3.10f, amb, diff, sTRUE);

        // Second grid pattern GlowRect
        GenTexture rect2, rect2x;
        rect2.Init(256, 256);
        LinearCombine(rect2, white, 1.0f, 0, 0); // white background
        GlowRect(rect2, rect2, gradBW, 0.5f, 0.5f, 0.36f, 0.0f, 0.0f, 0.20f, 0.8805f, 0.74f);

        rect2x.Init(256, 256);
        CoordMatrixTransform(rect2x, rect2, m3, WrapU | WrapV | FilterBilinear);

        // Multiply it over
        Paste(finalTex, finalTex, rect2x, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, CombineMultiply, 0);
    }
}

BENCHMARK(BM_Demo)->Repetitions(10)->Unit(benchmark::kMillisecond)->ComputeStatistics("min", [](const auto &v) { return *std::ranges::min_element(v); });

BENCHMARK_MAIN();