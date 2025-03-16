#include <openktg/pixel.h>
#include <openktg/types.h>

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
    for (int i = 0; i < 1024; ++i)
    {
        std::uint16_t r = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t g = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t b = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t a = static_cast<std::uint8_t>(random_uint16(0, 255));

        std::uint16_t s = random_uint16(0, 65535);

        // pixel * scalar
        pixel p(r, g, b, a);
        EXPECT_EQ(p * s, s * p);
        pixel ps = p * s;
        EXPECT_EQ(ps.r(), ::clamp_mult((r << 8) | r, s));
        EXPECT_EQ(ps.g(), ::clamp_mult((g << 8) | g, s));
        EXPECT_EQ(ps.b(), ::clamp_mult((b << 8) | b, s));
        EXPECT_EQ(ps.a(), ::clamp_mult((a << 8) | a, s));

        // pixel * pixel
        std::uint16_t rr = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t gg = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t bb = static_cast<std::uint8_t>(random_uint16(0, 255));
        std::uint16_t aa = static_cast<std::uint8_t>(random_uint16(0, 255));

        pixel pp(rr, gg, bb, aa);
        EXPECT_EQ(p * pp, pp * p);
        pixel pxpp = p * pp;
        EXPECT_EQ(pxpp.r(), ut::mult_intens((r << 8) | r, (rr << 8) | rr));
        EXPECT_EQ(pxpp.g(), ut::mult_intens((g << 8) | g, (gg << 8) | gg));
        EXPECT_EQ(pxpp.b(), ut::mult_intens((b << 8) | b, (bb << 8) | bb));
        EXPECT_EQ(pxpp.a(), ut::mult_intens((a << 8) | a, (aa << 8) | aa));

        // pixel - itself == 0
        pixel p0(0, 0, 0, 0);
        EXPECT_EQ(p - p, p0);

        // inverse
        EXPECT_EQ(~(~p), p);
        pixel pmax(0xFF, 0xFF, 0xFF, 0xFF);
        EXPECT_EQ(~p + p, pmax);
        EXPECT_EQ(pmax - p, ~p);
        EXPECT_EQ(pmax - ~p, p);

        // pixel + pixel
        EXPECT_EQ(p + pp, pp + p);
    }
}