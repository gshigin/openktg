#pragma once

#include <cstdint>

namespace openktg
{   
    class alignas(std::uint64_t) pixel 
    {
    public:
        pixel() = default;
        pixel(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
        pixel(std::uint32_t rgba); // from 0xaarrggbb (D3D style)

        // operator uint64_t() const; // cast to 0xaarrggbb

        pixel& lerp (pixel, uint16_t); // t=0..65536

        pixel& compositeAdd(pixel);
        pixel& compositeMulC(pixel);
        pixel& compositeROver(pixel);
        pixel& compositeScreen(pixel);
    private:
        // OpenGL byte order
        std::uint16_t r_;
        std::uint16_t g_;
        std::uint16_t b_;
        std::uint16_t a_;
    };

    pixel lerp(pixel lhs, pixel rhs, uint16_t t); // t=0..65536
    pixel compositeAdd(pixel lhs, pixel rhs);
    pixel compositeMulC(pixel lhs, pixel rhs);
    pixel compositeROver(pixel lhs, pixel rhs);
    pixel compositeScreen(pixel lhs, pixel rhs);
}