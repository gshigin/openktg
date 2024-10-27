#include <openktg/types.h>
#include <openktg/utility.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

using namespace openktg;

namespace legacy
{
    static int Lerp(int t,int a,int b)
    {
        return a + ((t * (b-a)) >> 16);
    }

    static std::uint32_t MulIntens(std::uint32_t a,std::uint32_t b)
    {
        std::uint32_t x = a*b + 0x8000;
        return (x + (x >> 16)) >> 16;
    }
}

TEST_CASE("Test lerp", "[utils]") {
    SECTION("Range") {
        std::uint32_t t = GENERATE(range(0, 65536, 4096));
        std::uint16_t a = GENERATE(range(0, 65535, 4096));
        std::uint16_t b = GENERATE(range(0, 65535, 4096));
        REQUIRE(static_cast<std::uint16_t>(legacy::Lerp(t, a, b)) == utility::lerp(a, b, t));
        REQUIRE(utility::lerp(b, a, 0x10000 - t) == utility::lerp(a, b, t));
    }
    SECTION("Random") {
        std::uint32_t t = GENERATE(take(16, random(0, 65536)));
        std::uint16_t a = GENERATE(take(16, random(0, 65535)));
        std::uint16_t b = GENERATE(take(16, random(0, 65535)));
        REQUIRE(static_cast<std::uint16_t>(legacy::Lerp(t, a, b)) == utility::lerp(a, b, t));
        REQUIRE(utility::lerp(b, a, 0x10000 - t) == utility::lerp(a, b, t));
    }
}

TEST_CASE("Test mult_intens", "[utils]") {
    SECTION("Range") {
        std::uint16_t a = GENERATE(range(0, 65535, 4096));
        std::uint16_t b = GENERATE(range(0, 65535, 4096));
        REQUIRE(static_cast<std::uint16_t>(legacy::MulIntens(a, b)) == utility::mult_intens(a, b));
        REQUIRE(utility::mult_intens(a, b) == utility::mult_intens(b, a));
    }
    SECTION("Random") {
        std::uint16_t a = GENERATE(take(16, random(0, 65535)));
        std::uint16_t b = GENERATE(take(16, random(0, 65535)));
        REQUIRE(static_cast<std::uint16_t>(legacy::MulIntens(a, b)) == utility::mult_intens(a, b));
        REQUIRE(utility::mult_intens(a, b) == utility::mult_intens(b, a));
    }
}