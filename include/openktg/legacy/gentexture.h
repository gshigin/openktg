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