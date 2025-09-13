#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-math.hpp"

#include <cstddef>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;

inline constexpr float COORD_SIZE{ 16.0 };

struct Coords
{
    size_t x{ 0 };
    size_t y{ 0 };

    constexpr Coords(size_t x, size_t y);

    constexpr operator rl::Vector2() const; // NOLINT(hicpp-explicit-conversions)
    constexpr operator sm::Vec2() const;    // NOLINT(hicpp-explicit-conversions)
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
constexpr Coords::Coords(const size_t x, const size_t y)
    : x{ x }
    , y{ y }
{
}

constexpr Coords::operator rl::Vector2() const
{
    return { static_cast<float>(x) * COORD_SIZE, -static_cast<float>(y) * COORD_SIZE };
}

constexpr Coords::operator sm::Vec2() const
{
    return { static_cast<float>(x) * COORD_SIZE, -static_cast<float>(y) * COORD_SIZE };
}
} // namespace seb_engine

#endif
