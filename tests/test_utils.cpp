#include <gtest/gtest.h>

#include <random>

#include <openktg/core/types.h>
#include <openktg/util/utility.h>

using namespace openktg;
namespace
{
namespace legacy
{
static auto Lerp(int t, int a, int b) -> int
{
    return a + ((t * (b - a)) >> 16);
}

static auto MulIntens(std::uint32_t a, std::uint32_t b) -> std::uint32_t
{
    std::uint32_t x = a * b + 0x8000;
    return (x + (x >> 16)) >> 16;
}
} // namespace legacy

auto random_uint16(std::uint32_t min, std::uint32_t max) -> std::uint16_t
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint16_t> dist(min, max);
    return static_cast<std::uint16_t>(dist(gen));
}
} // namespace

TEST(UtilsTest, LerpRange)
{
    for (int i = 0; i < 64; ++i)
    {
        for (int j = 0; j < 64; ++j)
        {
            for (int k = 0; k < 64; ++k)
            {
                auto t = static_cast<std::uint32_t>(i * 1024);
                auto a = static_cast<std::uint16_t>(j * 1024 + 3);
                auto b = static_cast<std::uint16_t>(k * 1024 - 7);

                EXPECT_EQ(static_cast<std::uint16_t>(legacy::Lerp(t, a, b)), util::lerp(a, b, t));
                EXPECT_EQ(util::lerp(b, a, 0x10000 - t), util::lerp(a, b, t));
            }
        }
    }
}

TEST(UtilsTest, LerpRandom)
{
    for (int i = 0; i < 1024; ++i)
    {
        std::uint32_t t = random_uint16(0, 65536);
        std::uint16_t a = random_uint16(0, 65535);
        std::uint16_t b = random_uint16(0, 65535);
        EXPECT_EQ(static_cast<std::uint16_t>(legacy::Lerp(t, a, b)), util::lerp(a, b, t));
        EXPECT_EQ(util::lerp(b, a, 0x10000 - t), util::lerp(a, b, t));
    }
}

TEST(UtilsTest, MulIntensRange)
{
    for (int i = 0; i < 64; ++i)
    {
        for (int j = 0; j < 64; ++j)
        {
            for (int k = 0; k < 64; ++k)
            {
                auto a = static_cast<std::uint16_t>(j * 1024 - 11);
                auto b = static_cast<std::uint16_t>(k * 1024 + 7);

                EXPECT_EQ(static_cast<std::uint16_t>(legacy::MulIntens(a, b)), util::mul_intens(a, b));
                EXPECT_EQ(util::mul_intens(a, b), util::mul_intens(b, a));
            }
        }
    }
}

TEST(UtilsTest, MulIntensRandom)
{
    for (int i = 0; i < 1024; ++i)
    {
        std::uint16_t a = random_uint16(0, 65535);
        std::uint16_t b = random_uint16(0, 65535);
        EXPECT_EQ(static_cast<std::uint16_t>(legacy::MulIntens(a, b)), util::mul_intens(a, b));
        EXPECT_EQ(util::mul_intens(a, b), util::mul_intens(b, a));
    }
}