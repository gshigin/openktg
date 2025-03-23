#include "gtest/gtest.h"
#include <openktg/pixel.h>
#include <openktg/types.h>
#include <openktg/utility.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <random>

using namespace openktg;
namespace ut = utility;

namespace
{
auto clamp_mult(std::uint16_t a, std::uint16_t b) -> std::uint16_t
{
    return std::clamp<std::uint32_t>(static_cast<std::uint32_t>(a) * static_cast<std::uint32_t>(b), 0, 65535);
}

auto random_uint16(std::uint16_t min, std::uint16_t max) -> std::uint16_t
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint16_t> dist(min, max);
    return dist(gen);
}
} // namespace

TEST(PixelTest, RandomOperations)
{
    using namespace openktg;

    // ctors
    EXPECT_EQ(pixel{0xFFABCDEF_argb}, pixel(0xAB_r, 0xCD_g, 0xEF_b, 0xFF_a));
    EXPECT_EQ(pixel{0xFFFFABABCDCDEFEF_argb64}, pixel(0xABAB_r, 0xCDCD_g, 0xEFEF_b, 0xFFFF_a));

    for (int i = 0; i < 1024; ++i)
    {
        std::uint16_t r = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t g = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t b = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t a = static_cast<std::uint8_t>(random_uint16(0, 255));

        std::uint16_t s = random_uint16(0, 65535);

        // pixel * scalar
        pixel p(static_cast<openktg::red8_t>(r), static_cast<openktg::green8_t>(g), static_cast<openktg::blue8_t>(b), static_cast<openktg::alpha8_t>(a));
        EXPECT_EQ(p * s, s * p);
        pixel ps = p * s;
        EXPECT_EQ(ps.r(), ut::mul_intens((r << 8) | r, s));
        EXPECT_EQ(ps.g(), ut::mul_intens((g << 8) | g, s));
        EXPECT_EQ(ps.b(), ut::mul_intens((b << 8) | b, s));
        EXPECT_EQ(ps.a(), ut::mul_intens((a << 8) | a, s));

        // pixel * pixel
        std::uint16_t rr = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t gg = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t bb = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t aa = static_cast<std::uint8_t>(random_uint16(0, 255));

        pixel pp(static_cast<openktg::red8_t>(rr), static_cast<openktg::green8_t>(gg), static_cast<openktg::blue8_t>(bb), static_cast<openktg::alpha8_t>(aa));
        EXPECT_EQ(p * pp, pp * p);
        pixel pxpp = p * pp;
        EXPECT_EQ(pxpp.r(), ut::mul_intens((r << 8) | r, (rr << 8) | rr));
        EXPECT_EQ(pxpp.g(), ut::mul_intens((g << 8) | g, (gg << 8) | gg));
        EXPECT_EQ(pxpp.b(), ut::mul_intens((b << 8) | b, (bb << 8) | bb));
        EXPECT_EQ(pxpp.a(), ut::mul_intens((a << 8) | a, (aa << 8) | aa));

        // pixel + pixel
        EXPECT_EQ(p + pp, pp + p);
        pixel pppp = p + pp;
        EXPECT_EQ(pppp.r(), std::min(((r << 8) | r) + ((rr << 8) | rr), 65535));
        EXPECT_EQ(pppp.g(), std::min(((g << 8) | g) + ((gg << 8) | gg), 65535));
        EXPECT_EQ(pppp.b(), std::min(((b << 8) | b) + ((bb << 8) | bb), 65535));
        EXPECT_EQ(pppp.a(), std::min(((a << 8) | a) + ((aa << 8) | aa), 65535));

        // max(pixel, pixel)
        EXPECT_EQ(p | pp, pp | p);
        pixel pmax = p | pp;
        EXPECT_EQ(pmax.r(), std::max(((r << 8) | r), ((rr << 8) | rr)));
        EXPECT_EQ(pmax.g(), std::max(((g << 8) | g), ((gg << 8) | gg)));
        EXPECT_EQ(pmax.b(), std::max(((b << 8) | b), ((bb << 8) | bb)));
        EXPECT_EQ(pmax.a(), std::max(((a << 8) | a), ((aa << 8) | aa)));

        // pixel + pixel
        EXPECT_EQ(p & pp, pp & p);
        pixel pmin = p & pp;
        EXPECT_EQ(pmin.r(), std::min(((r << 8) | r), ((rr << 8) | rr)));
        EXPECT_EQ(pmin.g(), std::min(((g << 8) | g), ((gg << 8) | gg)));
        EXPECT_EQ(pmin.b(), std::min(((b << 8) | b), ((bb << 8) | bb)));
        EXPECT_EQ(pmin.a(), std::min(((a << 8) | a), ((aa << 8) | aa)));

        // pixel - itself == 0
        pixel p_zero(0_r, 0_g, 0_b, 0_a);
        EXPECT_EQ(p - p, p_zero);

        // inverse
        EXPECT_EQ(~(~p), p);
        pixel p_black(0xFF_r, 0xFF_g, 0xFF_b, 0xFF_a);
        EXPECT_EQ(~p + p, p_black);
        EXPECT_EQ(p_black - p, ~p);
        EXPECT_EQ(p_black - ~p, p);
    }
}