/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/
#pragma once

#include "openktg/core/pixel.h"
#include <openktg/core/matrix.h>
#include <openktg/core/types.h>

// fwd
namespace openktg::inline core
{
struct pixel;
template struct matrix44<float>;
} // namespace openktg::inline core

// CellCenter. 2D pair of coordinates plus a cell color.
struct CellCenter
{
    sF32 x, y;
    openktg::pixel color;
};

// LinearInput. One input for "linear combine".
struct GenTexture;

struct LinearInput
{
    const GenTexture *Tex; // the input texture
    sF32 Weight;           // its weight
    sF32 UShift, VShift;   // u/v translate parameter
    sInt FilterMode;       // filtering mode (as in CoordMatrixTransform)
};

// X increases from 0 (left) to 1 (right)
// Y increases from 0 (bottom) to 1 (top)

struct GenTexture
{
    [[nodiscard]] std::uint32_t shift_x() const noexcept
    {
        return ShiftX;
    }
    [[nodiscard]] std::uint32_t shift_y() const noexcept
    {
        return ShiftY;
    }
    [[nodiscard]] std::uint32_t min_x() const noexcept
    {
        return MinX;
    }
    [[nodiscard]] std::uint32_t min_y() const noexcept
    {
        return MinY;
    }

    [[nodiscard]] auto width() const noexcept -> uint32_t
    {
        return XRes;
    }
    [[nodiscard]] auto height() const noexcept -> uint32_t
    {
        return YRes;
    }

    openktg::pixel &at(uint32_t x, uint32_t y)
    {
        return *(Data + (y << shift_x()) + x);
    }
    [[nodiscard]] const openktg::pixel &at(uint32_t x, uint32_t y) const
    {
        return *(Data + (y << shift_x()) + x);
    }
    openktg::pixel *data() noexcept
    {
        return Data;
    }
    [[nodiscard]] const openktg::pixel *data() const noexcept
    {
        return Data;
    }

    openktg::pixel *Data; // pointer to pixel data.
    sInt XRes;            // width of texture (must be a power of 2)
    sInt YRes;            // height of texture (must be a power of 2)
    sInt NPixels;         // width*height (number of pixels)

    sInt ShiftX; // log2(XRes)
    sInt ShiftY; // log2(YRes)
    sInt MinX;   // (1 << 24) / (2 * XRes) = Min X for clamp to edge
    sInt MinY;   // (1 << 24) / (2 * YRes) = Min Y for clamp to edge

    GenTexture();
    GenTexture(sInt xres, sInt yres);
    GenTexture(const GenTexture &x);
    ~GenTexture();

    void Init(sInt xres, sInt yres);
    void UpdateSize();
    void Swap(GenTexture &x);

    auto operator=(const GenTexture &x) -> GenTexture &;
};

[[nodiscard]] auto SizeMatchesWith(const GenTexture &x, const GenTexture &y) -> sBool;

// Sampling helpers with filtering (coords are 1.7.24 fixed point)
void SampleNearest(const GenTexture &input, openktg::core::pixel &result, sInt x, sInt y, sInt wrapMode);
void SampleBilinear(const GenTexture &input, openktg::core::pixel &result, sInt x, sInt y, sInt wrapMode);
void SampleFiltered(const GenTexture &input, openktg::core::pixel &result, sInt x, sInt y, sInt filterMode);
void SampleGradient(const GenTexture &input, openktg::core::pixel &result, sInt x);

// Ternary operations
enum TernaryOp
{
    TernaryLerp = 0, // (1-c.r) * a + c.r * b
    TernarySelect,
};

// Derive operations
enum DeriveOp
{
    DeriveGradient = 0,
    DeriveNormals,
};

// Combine operations
enum CombineOp
{
    // simple arithmetic
    CombineAdd = 0,  // x=saturate(a+b)
    CombineSub,      // x=saturate(a-b)
    CombineMulC,     // x=a*b
    CombineMin,      // x=min(a,b)
    CombineMax,      // x=max(a,b)
    CombineSetAlpha, // x.rgb=a.rgb, x.a=b.r
    CombinePreAlpha, // x.rgb=a.rgb*b.r, x.a=b.r

    CombineOver, // x=b over a
    CombineMultiply,
    CombineScreen,
    CombineDarken,
    CombineLighten,
};

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

// Filter mode
enum FilterMode
{
    WrapU = 0,  // wrap in u direction
    ClampU = 1, // clamp (to edge) in u direction

    WrapV = 0,  // wrap in v direction
    ClampV = 2, // clamp (to edge) in v direction

    FilterNearest = 0,  // nearest neighbor (point sampling)
    FilterBilinear = 4, // bilinear filtering.
};

// Actual generator functions
void Noise(GenTexture &input, const GenTexture &grad, sInt freqX, sInt freqY, sInt oct, sF32 fadeoff, sInt seed, sInt mode);
void GlowRect(GenTexture &input, const GenTexture &background, const GenTexture &grad, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx, sF32 vy, sF32 rectu,
              sF32 rectv);
void Cells(GenTexture &input, const GenTexture &grad, const CellCenter *centers, sInt nCenters, sF32 amp, sInt mode);

// Filters
void ColorMatrixTransform(GenTexture &input, const GenTexture &in, const openktg::matrix44<float> &matrix, sBool clampPremult);
void CoordMatrixTransform(GenTexture &input, const GenTexture &in, const openktg::matrix44<float> &matrix, sInt filterMode);
void ColorRemap(GenTexture &input, const GenTexture &in, const GenTexture &mapR, const GenTexture &mapG, const GenTexture &mapB);
void CoordRemap(GenTexture &input, const GenTexture &in, const GenTexture &remap, sF32 strengthU, sF32 strengthV, sInt filterMode);
void Derive(GenTexture &input, const GenTexture &in, DeriveOp op, sF32 strength);
void Blur(GenTexture &input, const GenTexture &in, sF32 sizex, sF32 sizey, sInt order, sInt mode);

// Combiners
void Ternary(GenTexture &input, const GenTexture &in1, const GenTexture &in2, const GenTexture &in3, TernaryOp op);
void Paste(GenTexture &input, const GenTexture &background, const GenTexture &snippet, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx, sF32 vy, CombineOp op,
           sInt mode);
void Bump(GenTexture &input, const GenTexture &surface, const GenTexture &normals, const GenTexture *specular, const GenTexture *falloff, sF32 px, sF32 py,
          sF32 pz, sF32 dx, sF32 dy, sF32 dz, const openktg::pixel &ambient, const openktg::pixel &diffuse, sBool directional);
void LinearCombine(GenTexture &input, const openktg::pixel &color, sF32 constWeight, const LinearInput *inputs, sInt nInputs);