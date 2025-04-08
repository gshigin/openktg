/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/
#pragma once

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/core/types.h>

// fwd
namespace openktg::inline core
{
class pixel;
class texture;
template struct matrix44<float>;
} // namespace openktg::inline core

// CellCenter. 2D pair of coordinates plus a cell color.
struct CellCenter
{
    sF32 x, y;
    openktg::pixel color;
};

struct LinearInput
{
    const openktg::texture *Tex; // the input texture
    sF32 Weight;                 // its weight
    sF32 UShift, VShift;         // u/v translate parameter
    sInt FilterMode;             // filtering mode (as in CoordMatrixTransform)
};

// X increases from 0 (left) to 1 (right)
// Y increases from 0 (bottom) to 1 (top)

[[nodiscard]] auto SizeMatchesWith(const openktg::texture &x, const openktg::texture &y) -> sBool;

// Sampling helpers with filtering (coords are 1.7.24 fixed point)
void SampleNearest(const openktg::texture &input, openktg::pixel &result, sInt x, sInt y, sInt wrapMode);
void SampleBilinear(const openktg::texture &input, openktg::pixel &result, sInt x, sInt y, sInt wrapMode);
void SampleFiltered(const openktg::texture &input, openktg::pixel &result, sInt x, sInt y, sInt filterMode);
void SampleGradient(const openktg::texture &input, openktg::pixel &result, sInt x);

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
void Noise(openktg::texture &input, const openktg::texture &grad, sInt freqX, sInt freqY, sInt oct, sF32 fadeoff, sInt seed, sInt mode);
void GlowRect(openktg::texture &input, const openktg::texture &background, const openktg::texture &grad, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx,
              sF32 vy, sF32 rectu, sF32 rectv);
void Cells(openktg::texture &input, const openktg::texture &grad, const CellCenter *centers, sInt nCenters, sF32 amp, sInt mode);

// Filters
void ColorMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, sBool clampPremult);
void CoordMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, sInt filterMode);
void ColorRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &mapR, const openktg::texture &mapG, const openktg::texture &mapB);
void CoordRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &remap, sF32 strengthU, sF32 strengthV, sInt filterMode);
void Derive(openktg::texture &input, const openktg::texture &in, DeriveOp op, sF32 strength);
void Blur(openktg::texture &input, const openktg::texture &in, sF32 sizex, sF32 sizey, sInt order, sInt mode);

// Combiners
void Ternary(openktg::texture &input, const openktg::texture &in1, const openktg::texture &in2, const openktg::texture &in3, TernaryOp op);
void Paste(openktg::texture &input, const openktg::texture &background, const openktg::texture &snippet, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx,
           sF32 vy, CombineOp op, sInt mode);
void Bump(openktg::texture &input, const openktg::texture &surface, const openktg::texture &normals, const openktg::texture *specular,
          const openktg::texture *falloff, sF32 px, sF32 py, sF32 pz, sF32 dx, sF32 dy, sF32 dz, const openktg::pixel &ambient, const openktg::pixel &diffuse,
          sBool directional);
void LinearCombine(openktg::texture &input, const openktg::pixel &color, sF32 constWeight, const LinearInput *inputs, sInt nInputs);