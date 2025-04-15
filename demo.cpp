/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/tex/composite.h>
#include <openktg/tex/procedural.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/utility.h>

#include <array>
#include <chrono>
#include <fstream>
#include <ratio>
#include <string>
#include <vector>

// Save an image as .TGA file
auto SaveImage(openktg::texture &img, const char *filename) -> bool
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        return false;
    }

    // prepare header
    std::array<std::uint8_t, 18> header = {};
    header.fill(0);

    header[2] = 2;                    // image type code 2 (RGB, uncompressed)
    header[12] = img.width() & 0xff;  // width (low byte)
    header[13] = img.width() >> 8;    // width (high byte)
    header[14] = img.height() & 0xff; // height (low byte)
    header[15] = img.height() >> 8;   // height (high byte)
    header[16] = 32;                  // pixel size (bits)

    // write header
    file.write(reinterpret_cast<char *>(header.data()), header.size());

    // write image data
    std::vector<std::uint8_t> lineBuf(img.width() * 4);
    for (int32_t y = 0; y < img.height(); y++)
    {
        const openktg::pixel *in = &img.data()[y * img.width()];

        // convert a line of pixels (as simple as possible - no gamma correction
        // etc.)
        for (int32_t x = 0; x < img.width(); x++)
        {
            lineBuf[x * 4 + 0] = in->b() >> 8;
            lineBuf[x * 4 + 1] = in->g() >> 8;
            lineBuf[x * 4 + 2] = in->r() >> 8;
            lineBuf[x * 4 + 3] = in->a() >> 8;

            ++in;
        }

        // write to file
        file.write(reinterpret_cast<char *>(lineBuf.data()), lineBuf.size());
    }
    return true;
}

auto ReadImage(openktg::texture &img, const char *filename) -> bool
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
        for (int x = 0; x < img.width(); ++x)
        {
            *out = openktg::pixel{static_cast<openktg::red16_t>((lineBuf[x * 4 + 0] << 8) | lineBuf[x * 4 + 0]),
                                  static_cast<openktg::green16_t>((lineBuf[x * 4 + 1] << 8) | lineBuf[x * 4 + 1]),
                                  static_cast<openktg::blue16_t>((lineBuf[x * 4 + 2] << 8) | lineBuf[x * 4 + 2]),
                                  static_cast<openktg::alpha16_t>((lineBuf[x * 4 + 3] << 8) | lineBuf[x * 4 + 3])};
            ++out;
        }
    }

    return true;
}

int main()
{
    using namespace openktg;
    using namespace openktg::util::constants;

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

    if (!SaveImage(noise, "noise.tga"))
    {
        printf("Couldn't write 'noise.tga'!\n");
        return 1;
    }

    // 4 "random voronoi" textures with different minimum distances
    texture voro[4];
    static int32_t voroIntens[4] = {37, 42, 37, 37};
    static int32_t voroCount[4] = {90, 132, 240, 255};
    static float voroDist[4] = {0.125f, 0.063f, 0.063f, 0.063f};

    for (int32_t i = 0; i < 4; i++)
    {
        voro[i].resize(256, 256);
        RandomVoronoi(voro[i], gradWhite, voroIntens[i], voroCount[i], voroDist[i]);

        std::string name = "voron" + std::to_string(i) + ".tga";

        if (!SaveImage(voro[i], name.data()))
        {
            printf("Couldn't write 'voro.tga'!\n");
            return 1;
        }
    }

    // linear combination of them
    LinearInput inputs[4];
    for (int32_t i = 0; i < 4; i++)
    {
        inputs[i].Tex = &voro[i];
        inputs[i].Weight = 1.5f;
        inputs[i].UShift = 0.0f;
        inputs[i].VShift = 0.0f;
        inputs[i].FilterMode = WrapU | WrapV | FilterNearest;
    }

    texture baseTex(256, 256);
    LinearCombine(baseTex, black, 0.0f, inputs, 4);

    if (!SaveImage(baseTex, "base_linear_combine.tga"))
    {
        printf("Couldn't write 'base_linear_combine.tga'!\n");
        return 1;
    }

    // blur it
    Blur(baseTex, baseTex, 0.0074f, 0.0074f, 1, WrapU | WrapV);

    if (!SaveImage(baseTex, "base_linear_combine_blur.tga"))
    {
        printf("Couldn't write 'base_linear_combine_blur.tga'!\n");
        return 1;
    }

    // add a noise layer
    texture noiseLayer(256, 256);
    Noise(noiseLayer, LinearGradient(0xff000000, 0xff646464), 4, 4, 5, 0.995f, 3, NoiseDirect | NoiseNormalize | NoiseBandlimit);

    if (!SaveImage(noiseLayer, "noise_layer.tga"))
    {
        printf("Couldn't write 'noise_layer.tga'!\n");
        return 1;
    }

    Paste(baseTex, baseTex, noiseLayer, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, CombineAdd, 0);

    if (!SaveImage(baseTex, "base_plus_noise.tga"))
    {
        printf("Couldn't write 'base_plus_noise.tga'!\n");
        return 1;
    }

    // colorize it
    Colorize(baseTex, 0xff747d8e, 0xfff1feff);

    if (!SaveImage(baseTex, "colorized.tga"))
    {
        printf("Couldn't write 'colorized.tga'!\n");
        return 1;
    }

    // Create transform matrix for grid pattern
    auto m1 = matrix44<float>::translation(-0.5f, -0.5f, 0.0f);
    auto m2 = matrix44<float>::scale(3.0f * SQRT2F, 3.0f * SQRT2F, 1.0f);
    auto m3 = m2 * m1;
    m1 = matrix44<float>::rotation_z(0.125f * PI2F);
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

    if (!SaveImage(rect1, "rect1.tga"))
    {
        printf("Couldn't write 'rect1.tga'!\n");
        return 1;
    }

    if (!SaveImage(rect1x, "rect1x.tga"))
    {
        printf("Couldn't write 'rect1x.tga'!\n");
        return 1;
    }

    if (!SaveImage(rect1n, "rect1n.tga"))
    {
        printf("Couldn't write 'rect1n.tga'!\n");
        return 1;
    }

    // Apply as bump map
    texture finalTex(256, 256);
    openktg::pixel amb{0xff101010_argb};
    openktg::pixel diff{0xffffffff_argb};

    Bump(finalTex, baseTex, rect1n, 0, 0, 0.0f, 0.0f, 0.0f, -2.518f, 0.719f, -3.10f, amb, diff, true);

    if (!SaveImage(finalTex, "final.tga"))
    {
        printf("Couldn't write 'final.tga'!\n");
        return 1;
    }

    // Second grid pattern GlowRect
    texture rect2(256, 256), rect2x(256, 256);
    LinearCombine(rect2, white, 1.0f, 0, 0); // white background
    GlowRect(rect2, rect2, gradBW, 0.5f, 0.5f, 0.36f, 0.0f, 0.0f, 0.20f, 0.8805f, 0.74f);

    CoordMatrixTransform(rect2x, rect2, m3, WrapU | WrapV | FilterBilinear);

    if (!SaveImage(rect2, "rect2.tga"))
    {
        printf("Couldn't write 'rect2.tga'!\n");
        return 1;
    }

    if (!SaveImage(rect2x, "rect2x.tga"))
    {
        printf("Couldn't write 'rect2x.tga'!\n");
        return 1;
    }

    // Multiply it over
    Paste(finalTex, finalTex, rect2x, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, CombineMultiply, 0);

    if (!SaveImage(finalTex, "finalfinal.tga"))
    {
        printf("Couldn't write 'finalfinal.tga'!\n");
        return 1;
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    using milliseconds = std::chrono::duration<double, std::milli>;

    printf("%f ms/tex\n", std::chrono::duration_cast<milliseconds>(endTime - startTime).count());

    return 0;
}
