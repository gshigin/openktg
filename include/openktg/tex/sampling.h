#pragma once

#include <cstdint>

// fwd
namespace openktg::inline core
{
class texture;
class pixel;
} // namespace openktg::inline core

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

// Sampling helpers with filtering (coords are 1.7.24 fixed point)
void SampleNearest(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t wrapMode);
void SampleBilinear(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t wrapMode);
void SampleFiltered(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t filterMode);
void SampleGradient(const openktg::texture &input, openktg::pixel &result, int32_t x);