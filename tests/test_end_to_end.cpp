#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>

#include <openktg/gentexture.h>
#include <openktg/pixel.h>
#include <openktg/procedural.h>
#include <openktg/types.h>

auto ReadImage(GenTexture &img, const char *filename) -> bool
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        return false;
    }

    std::array<uint8_t, 18> header;
    file.read(reinterpret_cast<char *>(header.data()), header.size());

    if (header[2] != 2)
    {
        return false;
    }

    if (header[16] != 32)
    {
        return false;
    }

    int width = header[12] | (header[13] << 8);
    int height = header[14] | (header[15] << 8);

    img.Init(width, height);

    std::vector<uint8_t> lineBuf(img.XRes * 4);
    for (int y = 0; y < img.YRes; ++y)
    {
        file.read(reinterpret_cast<char *>(lineBuf.data()), lineBuf.size());

        openktg::pixel *out = &img.Data[y * img.XRes];
        for (int x = 0; x < img.XRes; ++x)
        {
            *out = openktg::pixel{static_cast<openktg::red16_t>((lineBuf[x * 4 + 2] << 8) | lineBuf[x * 4 + 0]),
                                  static_cast<openktg::green16_t>((lineBuf[x * 4 + 1] << 8) | lineBuf[x * 4 + 1]),
                                  static_cast<openktg::blue16_t>((lineBuf[x * 4 + 0] << 8) | lineBuf[x * 4 + 2]),
                                  static_cast<openktg::alpha16_t>((lineBuf[x * 4 + 3] << 8) | lineBuf[x * 4 + 3])};
            ++out;
        }
    }

    return true;
}

auto GenerateTexture() -> GenTexture
{
    using namespace openktg;
    // initialize generator
    InitTexgen();

    // colors
    openktg::pixel black{0xFF000000_argb};
    openktg::pixel white{0xFFFFFFFF_argb};

    auto startTime = std::chrono::high_resolution_clock::now();

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
    openktg::pixel amb{0xff101010_argb};
    openktg::pixel diff{0xffffffff_argb};

    finalTex.Init(256, 256);
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

    return finalTex;
}

TEST(EndToEndTest, Test)
{
    namespace fs = std::filesystem;

    GenTexture generated = GenerateTexture();
    GenTexture reference;

    fs::path test_file_path = fs::path(__FILE__).parent_path() / "data/end_to_end.tga";
    ASSERT_TRUE(ReadImage(reference, test_file_path.c_str()));

    ASSERT_EQ(generated.XRes, reference.XRes);
    ASSERT_EQ(generated.YRes, reference.YRes);
    ASSERT_EQ(generated.NPixels, reference.NPixels);
    for (auto i = 0; i < generated.NPixels; ++i)
    {
        const auto &gen_pixel = *(generated.Data + i);
        const auto &ref_pixel = *(reference.Data + i);

        ASSERT_EQ(gen_pixel.r() >> 8, ref_pixel.r() >> 8);
        ASSERT_EQ(gen_pixel.g() >> 8, ref_pixel.g() >> 8);
        ASSERT_EQ(gen_pixel.b() >> 8, ref_pixel.b() >> 8);
        ASSERT_EQ(gen_pixel.a() >> 8, ref_pixel.a() >> 8);
    }
}