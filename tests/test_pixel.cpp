#include <openktg/types.h>
#include <openktg/pixel.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <cstdint>
#include <random>

#include <iostream>

using namespace openktg;
namespace ut = utility;

namespace Catch {
    template <>
    struct StringMaker<openktg::pixel> {
        static auto convert(const openktg::pixel& p) -> std::string {
            std::stringstream stream;
            stream << "0x " << std::hex;
            stream << p.r() << ' ' << p.g() << ' ' << p.b() << ' ' << p.a();
            return stream.str();
        }
    };
}

namespace
{
    auto clamp_mult(std::uint16_t a, std::uint16_t b) -> std::uint16_t
    {
        return std::clamp<std::uint32_t>(static_cast<std::uint32_t>(a) * static_cast<std::uint32_t>(b), 0, 65535);
    }
}

TEST_CASE("Test pixel", "[pixel]") {
    std::default_random_engine gen;
    std::uniform_int_distribution<> dist(0, 65535);

    SECTION("Random")
    {
        for (int i = 0; i < 1024; ++i) {
            std::uint16_t r = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t g = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t b = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t a = static_cast<std::uint8_t>(dist(gen));

            std::uint16_t s = dist(gen);
            
            // pixel * scalar
            pixel p(r, g, b, a);
            REQUIRE((p * s) == (s * p));
            pixel ps = p * s;
            REQUIRE(ps.r() == ::clamp_mult((r << 8) | r, s));
            REQUIRE(ps.g() == ::clamp_mult((g << 8) | g, s));
            REQUIRE(ps.b() == ::clamp_mult((b << 8) | b, s));
            REQUIRE(ps.a() == ::clamp_mult((a << 8) | a, s));

            // pixel * pixel
            std::uint16_t rr = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t gg = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t bb = static_cast<std::uint8_t>(dist(gen));
            std::uint16_t aa = static_cast<std::uint8_t>(dist(gen));

            pixel pp(rr, gg, bb, aa);
            REQUIRE((p * pp) == (pp * p));
            pixel pxpp = p * pp;
            REQUIRE(pxpp.r() == ut::mult_intens((r << 8) | r, (rr << 8) | rr));
            REQUIRE(pxpp.g() == ut::mult_intens((g << 8) | g, (gg << 8) | gg));
            REQUIRE(pxpp.b() == ut::mult_intens((b << 8) | b, (bb << 8) | bb));
            REQUIRE(pxpp.a() == ut::mult_intens((a << 8) | a, (aa << 8) | aa));

            // pixel - itself == 0
            pixel p0(0, 0, 0, 0);
            REQUIRE(p - p == p0);

            // inverse
            REQUIRE(~(~p) == p);
            pixel pmax(0xFF,0xFF,0xFF,0xFF);
            REQUIRE(~p + p == pmax);
            REQUIRE(pmax - p == ~p);
            REQUIRE(pmax - ~p == p);

            // pixel + pixel
            REQUIRE(p + pp == pp + p);
        }
    }
}