#pragma once

#include <vector>

#include <openktg/core/pixel.h>

namespace openktg::inline texture
{

class texture
{
  public:
    texture() = default;
    texture(uint32_t width, uint32_t height);
    texture(uint32_t width, uint32_t height, const pixel &fill_value);

    texture(const texture &) = default;
    auto operator=(const texture &) -> texture & = default;
    texture(texture &&) noexcept = default;
    auto operator=(texture &&) noexcept -> texture & = default;
    ~texture() = default;

    auto at(uint32_t x, uint32_t y) -> pixel &;
    [[nodiscard]] auto at(uint32_t x, uint32_t y) const -> const pixel &;
    auto data() noexcept -> pixel *
    {
        return pixels_.data();
    }
    [[nodiscard]] auto data() const noexcept -> const pixel *
    {
        return pixels_.data();
    }

    [[nodiscard]] auto width() const noexcept -> uint32_t
    {
        return width_;
    }
    [[nodiscard]] auto height() const noexcept -> uint32_t
    {
        return height_;
    }
    [[nodiscard]] auto empty() const noexcept -> bool
    {
        return pixels_.empty();
    }

    void fill(const pixel &value);
    void resize(uint32_t new_width, uint32_t new_height);

  private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    std::vector<pixel> pixels_{};
};
} // namespace openktg::inline texture