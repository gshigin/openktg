#include <openktg/core/texture.h>
#include <openktg/util/utility.h>

namespace openktg::inline core
{

texture::texture(uint32_t xres, uint32_t yres)
    : width_(xres), height_(yres), data_{width_ * height_}, shift_x_(openktg::util::floor_log_2(width_)), shift_y_(openktg::util::floor_log_2(height_)),
      min_x_(1 << (24 - 1 - shift_x_)), min_y_(1 << (24 - 1 - shift_y_))
{
}

[[nodiscard]] auto texture::shift_x() const noexcept -> uint32_t
{
    return shift_x_;
}
[[nodiscard]] auto texture::shift_y() const noexcept -> uint32_t
{
    return shift_y_;
}
[[nodiscard]] auto texture::min_x() const noexcept -> uint32_t
{
    return min_x_;
}
[[nodiscard]] auto texture::min_y() const noexcept -> uint32_t
{
    return min_y_;
}

[[nodiscard]] auto texture::width() const noexcept -> uint32_t
{
    return width_;
}
[[nodiscard]] auto texture::height() const noexcept -> uint32_t
{
    return height_;
}

[[nodiscard]] auto texture::pixel_count() const noexcept -> uint32_t
{
    return width() * height();
}

auto texture::at(uint32_t x, uint32_t y) -> openktg::pixel &
{
    return data_[(y << shift_x()) + x];
}
[[nodiscard]] auto texture::at(uint32_t x, uint32_t y) const -> const openktg::pixel &
{
    return data_[(y << shift_x()) + x];
}
auto texture::data() noexcept -> openktg::pixel *
{
    return data_.data();
}
[[nodiscard]] auto texture::data() const noexcept -> const openktg::pixel *
{
    return data_.data();
}
void texture::resize(uint32_t new_width, uint32_t new_heigth)
{
    width_ = new_width;
    height_ = new_heigth;

    data_.resize(width_ * height_);

    shift_x_ = openktg::util::floor_log_2(width_);
    shift_y_ = openktg::util::floor_log_2(height_);

    min_x_ = 1 << (24 - 1 - shift_x_);
    min_y_ = 1 << (24 - 1 - shift_y_);
}
} // namespace openktg::inline core