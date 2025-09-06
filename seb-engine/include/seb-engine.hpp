#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstddef>

namespace seb_engine
{
namespace rl = raylib;

inline constexpr float COORD_SIZE{ 16.0 };

struct Coords
{
    size_t x{ 0 };
    size_t y{ 0 };

    constexpr Coords(size_t x, size_t y);

    operator rl::Vector2() const; // NOLINT(hicpp-explicit-conversions)
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
constexpr Coords::Coords(size_t x, size_t y) : x(x), y(y)
{
}
} // namespace seb_engine

#endif
