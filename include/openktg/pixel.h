#pragma once

#include <openktg/utility.h>

#include <cstdint>
#include <algorithm>

namespace openktg
{   
    class alignas(std::uint64_t) pixel 
    {
    public:
        pixel() = default;
        pixel(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
        pixel(std::uint32_t rgba); // from 0xaarrggbb (D3D style)

        std::uint16_t r() const { return r_; }
        std::uint16_t g() const { return g_; }
        std::uint16_t b() const { return b_; }
        std::uint16_t a() const { return a_; }

        // combineAdd
        pixel& operator+=(pixel);
        // combineDub
        pixel& operator-=(pixel);
        // mulIntens
        pixel& operator*=(pixel);
        // pixel * scalar
        pixel& operator*=(std::uint16_t);
        // inverse pixel
        pixel operator~();

        bool operator==(pixel);

        pixel& lerp (pixel, uint16_t); // t=0..65536
    private:
        // OpenGL byte order
        std::uint16_t r_;
        std::uint16_t g_;
        std::uint16_t b_;
        std::uint16_t a_;  
    };

    pixel operator+(pixel lhs, pixel rhs);
    pixel operator-(pixel lhs, pixel rhs);
    pixel operator*(pixel lhs, pixel rhs);
    pixel operator*(pixel lhs, std::uint16_t);
    pixel operator*(std::uint16_t, pixel lhs);

    pixel lerp(pixel lhs, pixel rhs, uint16_t t); // t=0..65536

    pixel compositeAdd(pixel lhs, pixel rhs);
    pixel compositeMulC(pixel lhs, pixel rhs);
    pixel compositeROver(pixel lhs, pixel rhs);
    pixel compositeScreen(pixel lhs, pixel rhs);
}