#pragma once

#include <cassert>
#include <random>

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/tex/filters.h>
#include <openktg/tex/generators.h>
#include <openktg/util/random.h>

// Create a simple linear gradient texture
// Input colors are 0xaarrggbb (not premultiplied!)
static auto LinearGradient(uint32_t startCol, uint32_t endCol) -> openktg::texture
{
    openktg::texture tex(2, 1);

    tex.at(0, 0) = openktg::pixel{static_cast<openktg::color32_t>(startCol)};
    tex.at(1, 0) = openktg::pixel{static_cast<openktg::color32_t>(endCol)};

    return tex;
}

// Create a pattern of randomly colored voronoi cells
static void RandomVoronoi(openktg::texture &dest, const openktg::texture &grad, int32_t intensity, int32_t maxCount, float minDist,
                          int32_t seed = 0x339195BCC564A1E3)
{
    assert(maxCount <= 256);
    CellCenter centers[256];

    openktg::random::xoshiro128ss rng{openktg::random::seed(seed), openktg::random::seed(openktg::random::seed(seed))};
    std::uniform_real_distribution<float> distr(0.0f, 1.0f);

    // generate random center points
    for (int32_t i = 0; i < maxCount; i++)
    {
        int intens = intensity * distr(rng);

        centers[i].x = distr(rng);
        centers[i].y = distr(rng);
        centers[i].color = {static_cast<openktg::red8_t>(intens), static_cast<openktg::green8_t>(intens), static_cast<openktg::blue8_t>(intens),
                            static_cast<openktg::alpha8_t>(255)};
    }

    // remove points too close together
    float minDistSq = minDist * minDist;
    for (int32_t i = 1; i < maxCount;)
    {
        float x = centers[i].x;
        float y = centers[i].y;

        // try to find a point closer than minDist
        int32_t j;
        for (j = 0; j < i; j++)
        {
            float dx = centers[j].x - x;
            float dy = centers[j].y - y;

            if (dx < 0.0f)
                dx += 1.0f;
            if (dy < 0.0f)
                dy += 1.0f;

            dx = std::min(dx, 1.0f - dx);
            dy = std::min(dy, 1.0f - dy);

            if (dx * dx + dy * dy < minDistSq) // point is too close, stop
                break;
        }

        if (j < i)                            // we found such a point
            centers[i] = centers[--maxCount]; // remove this one
        else                                  // accept this one
            i++;
    }

    // generate the image
    Cells(dest, grad, centers, maxCount, 0.0f, CellInner);
}

// Transforms a grayscale image to a colored one with a matrix transform
static void Colorize(openktg::texture &img, uint32_t startCol, uint32_t endCol)
{
    openktg::matrix44<float> m;
    openktg::pixel s{static_cast<openktg::color32_t>(startCol)};
    openktg::pixel e{static_cast<openktg::color32_t>(endCol)};

    // calculate matrix
    m.data.fill(0);
    m(0, 0) = (e.r() - s.r()) / 65535.0f;
    m(1, 1) = (e.g() - s.g()) / 65535.0f;
    m(2, 2) = (e.b() - s.b()) / 65535.0f;
    m(3, 3) = 1.0f;
    m(0, 3) = s.r() / 65535.0f;
    m(1, 3) = s.g() / 65535.0f;
    m(2, 3) = s.b() / 65535.0f;

    // transform
    ColorMatrixTransform(img, img, m, true);
}