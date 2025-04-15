#pragma once

#include <cstdint>

#include <openktg/util/concepts.h>

// fwd
namespace openktg::inline core
{
class texture;
template <arithmetic T> struct matrix44;
} // namespace openktg::inline core

// Derive operations
enum DeriveOp
{
    DeriveGradient = 0,
    DeriveNormals,
};

void ColorMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, bool clampPremult);
void CoordMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, int32_t filterMode);
void ColorRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &mapR, const openktg::texture &mapG, const openktg::texture &mapB);
void CoordRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &remap, float strengthU, float strengthV, int32_t filterMode);
void Derive(openktg::texture &input, const openktg::texture &in, DeriveOp op, float strength);
void Blur(openktg::texture &input, const openktg::texture &in, float sizex, float sizey, int32_t order, int32_t mode);