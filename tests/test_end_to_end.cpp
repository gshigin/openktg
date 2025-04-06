#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/types.h>
#include <openktg/legacy/gentexture.h>
#include <openktg/tex/procedural.h>

auto ReadImage(texture &img, const char *filename) -> bool
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

    img.resize(width, height);

    std::vector<uint8_t> lineBuf(img.width() * 4);
    for (int y = 0; y < img.height(); ++y)
    {
        file.read(reinterpret_cast<char *>(lineBuf.data()), lineBuf.size());

        openktg::pixel *out = &img.data()[y * img.width()];
        for (int x = 0; x < img.height(); ++x)
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

auto GenerateTexture() -> texture
{
    using namespace openktg;

    // colors
    openktg::pixel black{0xFF000000_argb};
    openktg::pixel white{0xFFFFFFFF_argb};

    auto startTime = std::chrono::high_resolution_clock::now();

    // create gradients
    texture gradBW = LinearGradient(0xff000000, 0xffffffff);
    texture gradWB = LinearGradient(0xffffffff, 0xff000000);
    texture gradWhite = LinearGradient(0xffffffff, 0xffffffff);

    // simple noise test texture
    texture noise(256, 256);
    Noise(noise, gradBW, 2, 2, 6, 0.5f, 123, NoiseDirect | NoiseBandlimit | NoiseNormalize);

    // 4 "random voronoi" textures with different minimum distances
    texture voro[4];
    static sInt voroIntens[4] = {37, 42, 37, 37};
    static sInt voroCount[4] = {90, 132, 240, 255};
    static sF32 voroDist[4] = {0.125f, 0.063f, 0.063f, 0.063f};

    for (sInt i = 0; i < 4; i++)
    {
        voro[i].resize(256, 256);
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

    texture baseTex(256, 256);
    LinearCombine(baseTex, black, 0.0f, inputs, 4);

    // blur it
    Blur(baseTex, baseTex, 0.0074f, 0.0074f, 1, WrapU | WrapV);

    // add a noise layer
    texture noiseLayer(256, 256);
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
    texture rect1(256, 256), rect1x(256, 256), rect1n(256, 256);
    LinearCombine(rect1, black, 1.0f, 0, 0); // black background
    GlowRect(rect1, rect1, gradWB, 0.5f, 0.5f, 0.41f, 0.0f, 0.0f, 0.25f, 0.7805f, 0.64f);

    CoordMatrixTransform(rect1x, rect1, m3, WrapU | WrapV | FilterBilinear);

    // Make a normalmap from it
    Derive(rect1n, rect1x, DeriveNormals, 2.5f);

    // Apply as bump map
    texture finalTex(256, 256);
    openktg::pixel amb{0xff101010_argb};
    openktg::pixel diff{0xffffffff_argb};

    Bump(finalTex, baseTex, rect1n, 0, 0, 0.0f, 0.0f, 0.0f, -2.518f, 0.719f, -3.10f, amb, diff, sTRUE);

    // Second grid pattern GlowRect
    texture rect2(256, 256), rect2x(256, 256);
    LinearCombine(rect2, white, 1.0f, 0, 0); // white background
    GlowRect(rect2, rect2, gradBW, 0.5f, 0.5f, 0.36f, 0.0f, 0.0f, 0.20f, 0.8805f, 0.74f);

    CoordMatrixTransform(rect2x, rect2, m3, WrapU | WrapV | FilterBilinear);

    // Multiply it over
    Paste(finalTex, finalTex, rect2x, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, CombineMultiply, 0);

    return finalTex;
}

TEST(EndToEndTest, Test)
{
    namespace fs = std::filesystem;

    texture generated = GenerateTexture();
    texture reference;

    fs::path test_file_path = fs::path(__FILE__).parent_path() / "data/end_to_end.tga";
    ASSERT_TRUE(ReadImage(reference, test_file_path.c_str()));

    ASSERT_EQ(generated.width(), reference.width());
    ASSERT_EQ(generated.height(), reference.height());
    ASSERT_EQ(generated.pixel_count(), reference.pixel_count());
    for (auto i = 0; i < generated.pixel_count(); ++i)
    {
        const auto &gen_pixel = *(generated.data() + i);
        const auto &ref_pixel = *(reference.data() + i);

        ASSERT_EQ(gen_pixel.r() >> 8, ref_pixel.r() >> 8);
        ASSERT_EQ(gen_pixel.g() >> 8, ref_pixel.g() >> 8);
        ASSERT_EQ(gen_pixel.b() >> 8, ref_pixel.b() >> 8);
        ASSERT_EQ(gen_pixel.a() >> 8, ref_pixel.a() >> 8);
    }
}