#pragma once

#include <cstdint>

#include <openktg/core/pixel.h>

// fwd
namespace openktg::inline core
{
class texture;
} // namespace openktg::inline core

// Noise mode
enum NoiseMode
{
    NoiseDirect = 0, // use noise(x,y) directly
    NoiseAbs = 1,    // use abs(noise(x,y))

    NoiseUnnorm = 0,    // unnormalized (no further scaling)
    NoiseNormalize = 2, // normalized (scale so values always fall into [0,1] with no clamping)

    NoiseWhite = 0,     // white noise function
    NoiseBandlimit = 4, // bandlimited (perlin-like) noise function
};

// Cell mode
enum CellMode
{
    CellInner = 0, // inner (distance to cell center)
    CellOuter = 1, // outer (distance to edge)
};

// CellCenter. 2D pair of coordinates plus a cell color.
struct CellCenter
{
    float x, y;
    openktg::pixel color;
};

// Actual generator functions
void Noise(openktg::texture &input, const openktg::texture &grad, int32_t freqX, int32_t freqY, int32_t oct, float fadeoff, int32_t seed, int32_t mode);
void GlowRect(openktg::texture &input, const openktg::texture &background, const openktg::texture &grad, float orgx, float orgy, float ux, float uy, float vx,
              float vy, float rectu, float rectv);
void Cells(openktg::texture &input, const openktg::texture &grad, const CellCenter *centers, int32_t nCenters, float amp, int32_t mode);
