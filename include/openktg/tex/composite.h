#pragma once

#include <cstdint>

// fwd
namespace openktg::inline core
{
class pixel;
class texture;
} // namespace openktg::inline core

struct LinearInput
{
    const openktg::texture *Tex; // the input texture
    float Weight;                // its weight
    float UShift, VShift;        // u/v translate parameter
    int32_t FilterMode;          // filtering mode (as in CoordMatrixTransform)
};

// Ternary operations
enum TernaryOp
{
    TernaryLerp = 0, // (1-c.r) * a + c.r * b
    TernarySelect,
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

void Ternary(openktg::texture &input, const openktg::texture &in1, const openktg::texture &in2, const openktg::texture &in3, TernaryOp op);
void Paste(openktg::texture &input, const openktg::texture &background, const openktg::texture &snippet, float orgx, float orgy, float ux, float uy, float vx,
           float vy, CombineOp op, int32_t mode);
void Bump(openktg::texture &input, const openktg::texture &surface, const openktg::texture &normals, const openktg::texture *specular,
          const openktg::texture *falloff, float px, float py, float pz, float dx, float dy, float dz, const openktg::pixel &ambient,
          const openktg::pixel &diffuse, bool directional);
void LinearCombine(openktg::texture &input, const openktg::pixel &color, float constWeight, const LinearInput *inputs, int32_t nInputs);