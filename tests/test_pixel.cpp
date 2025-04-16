#include <gtest/gtest.h>

#include <cstdint>
#include <random>

#include <openktg/core/pixel.h>
#include <openktg/util/utility.h>

using namespace openktg;
namespace ut = util;

namespace
{

auto random_uint16(std::uint16_t min, std::uint16_t max) -> std::uint16_t
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint16_t> dist(min, max);
    return dist(gen);
}
} // namespace

namespace openktg
{
void PrintTo(const pixel &pixel, std::ostream *os)
{
    *os << "RGBA: " << std::hex << pixel.r() << ' ' << pixel.g() << ' ' << pixel.b() << ' ' << pixel.a();
}
} // namespace openktg

TEST(PixelTest, Constructor32)
{
    // Arrange
    pixel p_argb{0xFFABCDEF_argb};
    pixel p_rgba{0xAB_r, 0xCD_g, 0xEF_b, 0xFF_a};

    // Act

    // Assert
    EXPECT_EQ(p_argb, p_rgba);
}

TEST(PixelTest, Constructor64)
{
    // Arrange
    pixel p_argb{0xFFFFABABCDCDEFEF_argb64};
    pixel p_rgba{0xABAB_r, 0xCDCD_g, 0xEFEF_b, 0xFFFF_a};

    // Act

    // Assert
    EXPECT_EQ(p_argb, p_rgba);
}

TEST(PixelTest, ConstructorBothARGB)
{
    // Arrange
    pixel p_rgba32{0xAB_r, 0xCD_g, 0xEF_b, 0xFF_a};
    pixel p_rgba64{0xABAB_r, 0xCDCD_g, 0xEFEF_b, 0xFFFF_a};

    // Act

    // Assert
    EXPECT_EQ(p_rgba32, p_rgba64);
}

TEST(PixelTest, ConstructorBothRGBA)
{
    // Arrange
    pixel p_argb32{0xFFABCDEF_argb};
    pixel p_argb64{0xFFFFABABCDCDEFEF_argb64};

    // Act

    // Assert
    EXPECT_EQ(p_argb32, p_argb64);
}

struct TestOnePixel : testing::TestWithParam<unsigned>
{
  protected:
    TestOnePixel() : gen(GetParam()), dist(0, 255)
    {
    }

    void SetUp() override
    {
        p = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                  static_cast<openktg::alpha8_t>(dist(gen))};
        s = dist(gen);
    }

    std::mt19937 gen;
    std::uniform_int_distribution<std::uint16_t> dist;

    pixel p;
    std::uint16_t s;
};

TEST_P(TestOnePixel, PixelScalarMultiply)
{
    // Arrange

    // Act
    pixel ps = p * s;

    // Assert
    EXPECT_EQ(p * s, s * p);
    EXPECT_EQ(ps.r(), ut::mul_intens((p.r() << 8) | p.r(), s));
    EXPECT_EQ(ps.g(), ut::mul_intens((p.g() << 8) | p.g(), s));
    EXPECT_EQ(ps.b(), ut::mul_intens((p.b() << 8) | p.b(), s));
    EXPECT_EQ(ps.a(), ut::mul_intens((p.a() << 8) | p.a(), s));
}

TEST_P(TestOnePixel, PixelNeg)
{
    // Arrange
    pixel p_not_ref = pixel{static_cast<red16_t>(~p.r()), static_cast<green16_t>(~p.g()), static_cast<blue16_t>(~p.b()), static_cast<alpha16_t>(~p.a())};

    // Act
    pixel p_not = ~p;

    // Assert
    ASSERT_EQ(p_not, p_not_ref);
    ASSERT_EQ(p, ~p_not);
}

TEST_P(TestOnePixel, PixelSubstractItself)
{
    // Arrange
    pixel p_zero_ref = pixel{0x0_argb};

    // Act
    pixel p_zero = p - p;

    // Assert
    ASSERT_EQ(p_zero, p_zero_ref);
}

TEST_P(TestOnePixel, PixelClampPremult)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(std::min(p.r(), p.a())), static_cast<green16_t>(std::min(p.g(), p.a())),
                        static_cast<blue16_t>(std::min(p.b(), p.a())), static_cast<alpha16_t>(p.a())};

    // Act

    // Assert
    ASSERT_EQ(p.clamp_premult(), p_ref);
}

INSTANTIATE_TEST_SUITE_P(Seeds, TestOnePixel, ::testing::Values(42, 1337, 2024, 314159, 12345678910));

struct TestTwoPixels : testing::TestWithParam<unsigned>
{
  protected:
    TestTwoPixels() : gen(GetParam()), dist(0, 255)
    {
    }

    void SetUp() override
    {
        p1 = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                   static_cast<openktg::alpha8_t>(dist(gen))};
        p2 = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                   static_cast<openktg::alpha8_t>(dist(gen))};
        s = dist(gen);
    }

    std::mt19937 gen;
    std::uniform_int_distribution<std::uint16_t> dist;

    pixel p1, p2;
    std::uint16_t s;
};

TEST_P(TestTwoPixels, PixelAdd)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(std::min(p1.r() + p2.r(), 65535)), static_cast<green16_t>(std::min(p1.g() + p2.g(), 65535)),
                        static_cast<blue16_t>(std::min(p1.b() + p2.b(), 65535)), static_cast<alpha16_t>(std::min(p1.a() + p2.a(), 65535))};

    // Act
    pixel p_add = p1 + p2;

    // Assert
    ASSERT_EQ(p1 + p2, p2 + p1);
    ASSERT_EQ(p_add, p_ref);
}

TEST_P(TestTwoPixels, PixelMult)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(ut::mul_intens(p1.r(), p2.r())), static_cast<green16_t>(ut::mul_intens(p1.g(), p2.g())),
                        static_cast<blue16_t>(ut::mul_intens(p1.b(), p2.b())), static_cast<alpha16_t>(ut::mul_intens(p1.a(), p2.a()))};

    // Act
    pixel p_mult = p1 * p2;

    // Assert
    ASSERT_EQ(p1 * p2, p2 * p1);
    ASSERT_EQ(p_mult, p_ref);
}

TEST_P(TestTwoPixels, PixelMax)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(std::max(p1.r(), p2.r())), static_cast<green16_t>(std::max(p1.g(), p2.g())),
                        static_cast<blue16_t>(std::max(p1.b(), p2.b())), static_cast<alpha16_t>(std::max(p1.a(), p2.a()))};

    // Act
    pixel p_max = p1 | p2;

    // Assert
    ASSERT_EQ(p1 | p2, p2 | p1);
    ASSERT_EQ(p_max, p_ref);
}

TEST_P(TestTwoPixels, PixelMin)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(std::min(p1.r(), p2.r())), static_cast<green16_t>(std::min(p1.g(), p2.g())),
                        static_cast<blue16_t>(std::min(p1.b(), p2.b())), static_cast<alpha16_t>(std::min(p1.a(), p2.a()))};

    // Act
    pixel p_min = p1 & p2;

    // Assert
    ASSERT_EQ(p1 & p2, p2 & p1);
    ASSERT_EQ(p_min, p_ref);
}

TEST_P(TestTwoPixels, PixelLerp)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(ut::lerp(p1.r(), p2.r(), s)), static_cast<green16_t>(ut::lerp(p1.g(), p2.g(), s)),
                        static_cast<blue16_t>(ut::lerp(p1.b(), p2.b(), s)), static_cast<alpha16_t>(ut::lerp(p1.a(), p2.a(), s))};

    // Act
    pixel p_lerp = lerp(p1, p2, s);

    // Assert
    ASSERT_EQ(lerp(p1, p2, s), lerp(p2, p1, 0x10000 - s));
    ASSERT_EQ(p_lerp, p_ref);
}

TEST_P(TestTwoPixels, PixelCombinePreAlpha)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(ut::mul_intens(p1.r(), p2.r())), static_cast<green16_t>(ut::mul_intens(p1.g(), p2.r())),
                        static_cast<blue16_t>(ut::mul_intens(p1.b(), p2.r())), static_cast<alpha16_t>(p2.g())};

    // Act
    pixel p_combine_pre_alpha = p1 * p2.r();
    p_combine_pre_alpha.set_alpha(static_cast<alpha16_t>(p2.g()));

    // Assert
    ASSERT_EQ(p_combine_pre_alpha, p_ref);
}

TEST_P(TestTwoPixels, PixelCombineOver)
{
    // Arrange
    pixel p_ref = pixel{
        static_cast<red16_t>(std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(0xFFFF - p2.a(), p1.r())) + static_cast<uint32_t>(p2.r()), 0xFFFF)),
        static_cast<green16_t>(std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(0xFFFF - p2.a(), p1.g())) + static_cast<uint32_t>(p2.g()), 0xFFFF)),
        static_cast<blue16_t>(std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(0xFFFF - p2.a(), p1.b())) + static_cast<uint32_t>(p2.b()), 0xFFFF)),
        static_cast<alpha16_t>(p1.a() + ut::mul_intens(p2.a(), 0xFFFF - p1.a()))};

    // Act
    pixel p_combine_over = combineOver(p2, p1);

    // Assert
    ASSERT_NE(combineOver(p1, p2), combineOver(p2, p1));
    ASSERT_EQ(p_combine_over, p_ref);
}

TEST_P(TestTwoPixels, PixelCombineMultiply)
{
    // Arrange
    std::uint32_t r32 =
        std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(~p1.a(), p2.r())) + static_cast<uint32_t>(ut::mul_intens(~p2.a(), p1.r())) +
                                    static_cast<uint32_t>(ut::mul_intens(p1.r(), p2.r())),
                                0xFFFF);
    std::uint32_t g32 =
        std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(~p1.a(), p2.g())) + static_cast<uint32_t>(ut::mul_intens(~p2.a(), p1.g())) +
                                    static_cast<uint32_t>(ut::mul_intens(p1.g(), p2.g())),
                                0xFFFF);
    std::uint32_t b32 =
        std::min<std::uint32_t>(static_cast<uint32_t>(ut::mul_intens(~p1.a(), p2.b())) + static_cast<uint32_t>(ut::mul_intens(~p2.a(), p1.b())) +
                                    static_cast<uint32_t>(ut::mul_intens(p1.b(), p2.b())),
                                0xFFFF);
    std::uint32_t a32 = std::min<std::uint32_t>(static_cast<uint32_t>(p2.a()) + static_cast<uint32_t>(ut::mul_intens(~p2.a(), p1.a())), 0xFFFF);
    pixel p_ref = pixel{static_cast<red16_t>(r32), static_cast<green16_t>(g32), static_cast<blue16_t>(b32), static_cast<alpha16_t>(a32)};

    // Act
    pixel p_combine_mult = combineMultiply(p1, p2);

    // Assert
    ASSERT_EQ(combineMultiply(p1, p2), combineMultiply(p2, p1));
    ASSERT_EQ(p_combine_mult, p_ref);
}

TEST_P(TestTwoPixels, PixelCombineScreen)
{
    // Arrange

    pixel p_ref = pixel{
        static_cast<red16_t>(p2.r() + ut::mul_intens(p1.r(), 0xFFFF - p2.r())), static_cast<green16_t>(p2.g() + ut::mul_intens(p1.g(), 0xFFFF - p2.g())),
        static_cast<blue16_t>(p2.b() + ut::mul_intens(p1.b(), 0xFFFF - p2.b())), static_cast<alpha16_t>(p2.a() + ut::mul_intens(p1.a(), 0xFFFF - p2.a()))};

    // Act
    pixel p_screen = combineScreen(p1, p2);

    // Assert
    ASSERT_EQ(combineScreen(p1, p2), combineScreen(p2, p1));
    ASSERT_EQ(p_screen, p_ref);
}

TEST_P(TestTwoPixels, PixelCombineDarken)
{
    // Arrange
    std::uint32_t r32 = std::min(p1.r() + p2.r() - std::max(ut::mul_intens(p1.r(), p2.a()), ut::mul_intens(p2.r(), p1.a())), 0xFFFF);
    std::uint32_t g32 = std::min(p1.g() + p2.g() - std::max(ut::mul_intens(p1.g(), p2.a()), ut::mul_intens(p2.g(), p1.a())), 0xFFFF);
    std::uint32_t b32 = std::min(p1.b() + p2.b() - std::max(ut::mul_intens(p1.b(), p2.a()), ut::mul_intens(p2.b(), p1.a())), 0xFFFF);
    std::uint32_t a32 = std::min(p2.a() + ut::mul_intens(p1.a(), 0xFFFF - p2.a()), 0xFFFF);
    pixel p_ref = pixel{static_cast<red16_t>(r32), static_cast<green16_t>(g32), static_cast<blue16_t>(b32), static_cast<alpha16_t>(a32)};

    // Act
    pixel p_darken = combineDarken(p1, p2);

    // Assert
    ASSERT_EQ(combineDarken(p1, p2), combineDarken(p2, p1));
    ASSERT_EQ(p_darken, p_ref);
}

TEST_P(TestTwoPixels, PixelCombineLighten)
{
    // Arrange
    std::uint32_t r32 = std::min(p1.r() + p2.r() - std::min(ut::mul_intens(p1.r(), p2.a()), ut::mul_intens(p2.r(), p1.a())), 0xFFFF);
    std::uint32_t g32 = std::min(p1.g() + p2.g() - std::min(ut::mul_intens(p1.g(), p2.a()), ut::mul_intens(p2.g(), p1.a())), 0xFFFF);
    std::uint32_t b32 = std::min(p1.b() + p2.b() - std::min(ut::mul_intens(p1.b(), p2.a()), ut::mul_intens(p2.b(), p1.a())), 0xFFFF);
    std::uint32_t a32 = std::min(p2.a() + ut::mul_intens(p1.a(), 0xFFFF - p2.a()), 0xFFFF);
    pixel p_ref = pixel{static_cast<red16_t>(r32), static_cast<green16_t>(g32), static_cast<blue16_t>(b32), static_cast<alpha16_t>(a32)};

    // Act
    pixel p_lighten = combineLighten(p1, p2);

    // Assert
    ASSERT_EQ(combineLighten(p1, p2), combineLighten(p2, p1));
    ASSERT_EQ(p_lighten, p_ref);
}

INSTANTIATE_TEST_SUITE_P(Seeds, TestTwoPixels, ::testing::Values(42, 1337, 2024, 314159, 12345678910));

struct TestThreePixels : testing::TestWithParam<unsigned>
{
  protected:
    TestThreePixels() : gen(GetParam()), dist(0, 255)
    {
    }

    void SetUp() override
    {
        p1 = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                   static_cast<openktg::alpha8_t>(dist(gen))};
        p2 = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                   static_cast<openktg::alpha8_t>(dist(gen))};
        p3 = pixel{static_cast<openktg::red8_t>(dist(gen)), static_cast<openktg::green8_t>(dist(gen)), static_cast<openktg::blue8_t>(dist(gen)),
                   static_cast<openktg::alpha8_t>(dist(gen))};
    }

    std::mt19937 gen;
    std::uniform_int_distribution<std::uint16_t> dist;

    pixel p1, p2, p3;
};

TEST_P(TestThreePixels, TernaryLerp)
{
    // Arrange
    pixel p_ref = pixel{static_cast<red16_t>(ut::mul_intens(0xFFFF - p3.r(), p1.r()) + ut::mul_intens(p3.r(), p2.r())),
                        static_cast<green16_t>(ut::mul_intens(0xFFFF - p3.r(), p1.g()) + ut::mul_intens(p3.r(), p2.g())),
                        static_cast<blue16_t>(ut::mul_intens(0xFFFF - p3.r(), p1.b()) + ut::mul_intens(p3.r(), p2.b())),
                        static_cast<alpha16_t>(ut::mul_intens(0xFFFF - p3.r(), p1.a()) + ut::mul_intens(p3.r(), p2.a()))};

    // Act
    pixel p_ternary_lerp = (~p3.r() * p1) + (p3.r() * p2);

    // Assert
    ASSERT_EQ(p_ternary_lerp, p_ref);
}

INSTANTIATE_TEST_SUITE_P(Seeds, TestThreePixels, ::testing::Values(42, 1337, 2024, 314159, 12345678910));