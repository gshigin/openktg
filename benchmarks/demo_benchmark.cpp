#include <benchmark/benchmark.h>

#include <openktg/gentexture.h>
#include <openktg/procedural.h>

static void BM_Demo(benchmark::State &state)
{
    for (auto _ : state)
    {
        // initialize generator
        InitTexgen();

        // colors
        Pixel black, white;
        black.Init(0, 0, 0, 255);
        white.Init(255, 255, 255, 255);
        // create gradients
        GenTexture gradBW = LinearGradient(0xff000000, 0xffffffff);
        GenTexture gradWB = LinearGradient(0xffffffff, 0xff000000);
        GenTexture gradWhite = LinearGradient(0xffffffff, 0xffffffff);

        // simple noise test texture
        GenTexture noise;
        noise.Init(256, 256);
        noise.Noise(gradBW, 2, 2, 6, 0.5f, 123, GenTexture::NoiseDirect | GenTexture::NoiseBandlimit | GenTexture::NoiseNormalize);

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
            inputs[i].FilterMode = GenTexture::WrapU | GenTexture::WrapV | GenTexture::FilterNearest;
        }

        GenTexture baseTex;
        baseTex.Init(256, 256);
        baseTex.LinearCombine(black, 0.0f, inputs, 4);

        // blur it
        baseTex.Blur(baseTex, 0.0074f, 0.0074f, 1, GenTexture::WrapU | GenTexture::WrapV);

        // add a noise layer
        GenTexture noiseLayer;
        noiseLayer.Init(256, 256);
        noiseLayer.Noise(LinearGradient(0xff000000, 0xff646464), 4, 4, 5, 0.995f, 3,
                         GenTexture::NoiseDirect | GenTexture::NoiseNormalize | GenTexture::NoiseBandlimit);

        baseTex.Paste(baseTex, noiseLayer, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, GenTexture::CombineAdd, 0);

        // colorize it
        Colorize(baseTex, 0xff747d8e, 0xfff1feff);

        // Create transform matrix for grid pattern
        Matrix44 m1, m2, m3;
        MatTranslate(m1, -0.5f, -0.5f, 0.0f);
        MatScale(m2, 3.0f * sSQRT2F, 3.0f * sSQRT2F, 1.0f);
        MatMult(m3, m2, m1);
        MatRotateZ(m1, 0.125f * sPI2F);
        MatMult(m2, m1, m3);
        MatTranslate(m1, 0.5f, 0.5f, 0.0f);
        MatMult(m3, m1, m2);

        // Grid pattern GlowRect
        GenTexture rect1, rect1x, rect1n;
        rect1.Init(256, 256);
        rect1.LinearCombine(black, 1.0f, 0, 0); // black background
        rect1.GlowRect(rect1, gradWB, 0.5f, 0.5f, 0.41f, 0.0f, 0.0f, 0.25f, 0.7805f, 0.64f);

        rect1x.Init(256, 256);
        rect1x.CoordMatrixTransform(rect1, m3, GenTexture::WrapU | GenTexture::WrapV | GenTexture::FilterBilinear);

        // Make a normalmap from it
        rect1n.Init(256, 256);
        rect1n.Derive(rect1x, GenTexture::DeriveNormals, 2.5f);

        // Apply as bump map
        GenTexture finalTex;
        Pixel amb, diff;

        finalTex.Init(256, 256);
        amb.Init(0xff101010);
        diff.Init(0xffffffff);
        finalTex.Bump(baseTex, rect1n, 0, 0, 0.0f, 0.0f, 0.0f, -2.518f, 0.719f, -3.10f, amb, diff, sTRUE);

        // Second grid pattern GlowRect
        GenTexture rect2, rect2x;
        rect2.Init(256, 256);
        rect2.LinearCombine(white, 1.0f, 0, 0); // white background
        rect2.GlowRect(rect2, gradBW, 0.5f, 0.5f, 0.36f, 0.0f, 0.0f, 0.20f, 0.8805f, 0.74f);

        rect2x.Init(256, 256);
        rect2x.CoordMatrixTransform(rect2, m3, GenTexture::WrapU | GenTexture::WrapV | GenTexture::FilterBilinear);

        // Multiply it over
        finalTex.Paste(finalTex, rect2x, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, GenTexture::CombineMultiply, 0);
    }
}

BENCHMARK(BM_Demo)->Repetitions(10)->Unit(benchmark::kMillisecond)->ComputeStatistics("min", [](const auto &v) { return *std::ranges::min_element(v); });

BENCHMARK_MAIN();