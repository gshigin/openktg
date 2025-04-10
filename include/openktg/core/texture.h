#pragma once

#include <cstdint>
#include <vector>

#include <openktg/core/pixel.h>

namespace openktg::inline core
{
class texture
{
  public:
    texture() = default;
    texture(uint32_t width, uint32_t height);

    [[nodiscard]] auto shift_x() const noexcept -> uint32_t;
    [[nodiscard]] auto shift_y() const noexcept -> uint32_t;

    [[nodiscard]] auto min_x() const noexcept -> uint32_t;
    [[nodiscard]] auto min_y() const noexcept -> uint32_t;

    [[nodiscard]] auto width() const noexcept -> uint32_t;
    [[nodiscard]] auto height() const noexcept -> uint32_t;

    [[nodiscard]] auto pixel_count() const noexcept -> uint32_t;

    auto at(uint32_t x, uint32_t y) -> openktg::pixel &;
    [[nodiscard]] auto at(uint32_t x, uint32_t y) const -> const openktg::pixel &;

    auto data() noexcept -> openktg::pixel *;
    [[nodiscard]] auto data() const noexcept -> const openktg::pixel *;

    void resize(uint32_t new_width, uint32_t new_heigth);

  private:
    uint32_t width_;
    uint32_t height_;

    std::vector<openktg::pixel> data_; // pixel data

    uint32_t shift_x_; // log2(width)
    uint32_t shift_y_; // log2(height)
    uint32_t min_x_;   // (1 << 24) / (2 * width) = Min X for clamp to edge
    uint32_t min_y_;   // (1 << 24) / (2 * height) = Min X for clamp to edge
};
} // namespace openktg::inline core