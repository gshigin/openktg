/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

#include "openktg/pixel.h"
#include <openktg/gentexture.h>
#include <openktg/procedural.h>

#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <ratio>
#include <string>
#include <vector>

// Save an image as .TGA file
auto SaveImage(GenTexture &img, const char *filename) -> bool
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        return false;
    }

    // prepare header
    std::array<std::uint8_t, 18> header = {};
    header.fill(0);

    header[2] = 2;                // image type code 2 (RGB, uncompressed)
    header[12] = img.XRes & 0xff; // width (low byte)
    header[13] = img.XRes >> 8;   // width (high byte)
    header[14] = img.YRes & 0xff; // height (low byte)
    header[15] = img.YRes >> 8;   // height (high byte)
    header[16] = 32;              // pixel size (bits)

    // write header
    file.write(reinterpret_cast<char *>(header.data()), header.size());

    // write image data
    std::vector<std::uint8_t> lineBuf(img.XRes * 4);
    for (sInt y = 0; y < img.YRes; y++)
    {
        const openktg::pixel *in = &img.Data[y * img.XRes];

        // convert a line of pixels (as simple as possible - no gamma correction
        // etc.)
        for (sInt x = 0; x < img.XRes; x++)
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
            *out = openktg::pixel{static_cast<openktg::red16_t>((lineBuf[x * 4 + 0] << 8) | lineBuf[x * 4 + 0]),
                                  static_cast<openktg::green16_t>((lineBuf[x * 4 + 1] << 8) | lineBuf[x * 4 + 1]),
                                  static_cast<openktg::blue16_t>((lineBuf[x * 4 + 2] << 8) | lineBuf[x * 4 + 2]),
                                  static_cast<openktg::alpha16_t>((lineBuf[x * 4 + 3] << 8) | lineBuf[x * 4 + 3])};
            // out->b = (lineBuf[x * 4 + 0] << 8) | lineBuf[x * 4 + 0];
            // out->g = (lineBuf[x * 4 + 1] << 8) | lineBuf[x * 4 + 1];
            // out->r = (lineBuf[x * 4 + 2] << 8) | lineBuf[x * 4 + 2];
            // out->a = (lineBuf[x * 4 + 3] << 8) | lineBuf[x * 4 + 3];
            ++out;
        }
    }

    return true;
}

int main()
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

    if (!SaveImage(noise, "noise.tga"))
    {
        printf("Couldn't write 'noise.tga'!\n");
        return 1;
    }

    // 4 "random voronoi" textures with different minimum distances
    GenTexture voro[4];
    static sInt voroIntens[4] = {37, 42, 37, 37};
    static sInt voroCount[4] = {90, 132, 240, 255};
    static sF32 voroDist[4] = {0.125f, 0.063f, 0.063f, 0.063f};

    for (sInt i = 0; i < 4; i++)
    {
        voro[i].Init(256, 256);
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

    if (!SaveImage(baseTex, "base_linear_combine.tga"))
    {
        printf("Couldn't write 'base_linear_combine.tga'!\n");
        return 1;
    }

    // blur it
    baseTex.Blur(baseTex, 0.0074f, 0.0074f, 1, GenTexture::WrapU | GenTexture::WrapV);

    if (!SaveImage(baseTex, "base_linear_combine_blur.tga"))
    {
        printf("Couldn't write 'base_linear_combine_blur.tga'!\n");
        return 1;
    }

    // add a noise layer
    GenTexture noiseLayer;
    noiseLayer.Init(256, 256);
    noiseLayer.Noise(LinearGradient(0xff000000, 0xff646464), 4, 4, 5, 0.995f, 3,
                     GenTexture::NoiseDirect | GenTexture::NoiseNormalize | GenTexture::NoiseBandlimit);

    if (!SaveImage(noiseLayer, "noise_layer.tga"))
    {
        printf("Couldn't write 'noise_layer.tga'!\n");
        return 1;
    }

    baseTex.Paste(baseTex, noiseLayer, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, GenTexture::CombineAdd, 0);

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
    GenTexture finalTex;
    openktg::pixel amb{0xff101010_argb};
    openktg::pixel diff{0xffffffff_argb};

    finalTex.Init(256, 256);
    // amb.Init(0xff101010);
    // diff.Init(0xffffffff);
    finalTex.Bump(baseTex, rect1n, 0, 0, 0.0f, 0.0f, 0.0f, -2.518f, 0.719f, -3.10f, amb, diff, sTRUE);

    if (!SaveImage(finalTex, "final.tga"))
    {
        printf("Couldn't write 'final.tga'!\n");
        return 1;
    }

    // Second grid pattern GlowRect
    GenTexture rect2, rect2x;
    rect2.Init(256, 256);
    rect2.LinearCombine(white, 1.0f, 0, 0); // white background
    rect2.GlowRect(rect2, gradBW, 0.5f, 0.5f, 0.36f, 0.0f, 0.0f, 0.20f, 0.8805f, 0.74f);

    rect2x.Init(256, 256);
    rect2x.CoordMatrixTransform(rect2, m3, GenTexture::WrapU | GenTexture::WrapV | GenTexture::FilterBilinear);

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
    finalTex.Paste(finalTex, rect2x, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, GenTexture::CombineMultiply, 0);

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
