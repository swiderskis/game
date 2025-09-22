#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-math.hpp"

#include <cstddef>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;

template <float CoordSize>
struct Coords
{
    size_t x{ 0 };
    size_t y{ 0 };

    constexpr Coords(size_t x, size_t y);

    operator rl::Vector2() const;        // NOLINT(hicpp-explicit-conversions)
    constexpr operator sm::Vec2() const; // NOLINT(hicpp-explicit-conversions)
    [[nodiscard]] auto operator+(Coords coords) const -> Coords;
    [[nodiscard]] auto operator-(Coords coords) const -> Coords;
    [[maybe_unused]] auto operator+=(Coords coords) -> Coords&;
    [[maybe_unused]] auto operator-=(Coords coords) -> Coords&;
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <float CoordSize>
constexpr Coords<CoordSize>::Coords(const size_t x, const size_t y)
    : x{ x }
    , y{ y }
{
}

template <float CoordSize>
Coords<CoordSize>::operator rl::Vector2() const
{
    return { static_cast<float>(x) * CoordSize, -static_cast<float>(y) * CoordSize };
}

template <float CoordSize>
constexpr Coords<CoordSize>::operator sm::Vec2() const
{
    return { static_cast<float>(x) * CoordSize, -static_cast<float>(y) * CoordSize };
}

template <float CoordSize>
auto Coords<CoordSize>::operator+(Coords coords) const -> Coords
{
    return { x + coords.x, y + coords.y };
}

template <float CoordSize>
auto Coords<CoordSize>::operator-(Coords coords) const -> Coords
{
    return { x - coords.x, y - coords.y };
}

template <float CoordSize>
auto Coords<CoordSize>::operator+=(Coords coords) -> Coords&
{
    x += coords.x;
    y += coords.y;

    return *this;
}

template <float CoordSize>
auto Coords<CoordSize>::operator-=(Coords coords) -> Coords&
{
    x -= coords.x;
    y -= coords.y;

    return *this;
}
} // namespace seb_engine

#endif
