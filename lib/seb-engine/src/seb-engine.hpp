#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seblib.hpp"

#include <cstddef>

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;

inline constexpr unsigned MAX_ENTITIES = 1024;

inline constexpr float TILE_SIZE = 16.0;

inline constexpr sl::SimpleVec2 TILE_CBOX_SIZE{ TILE_SIZE, TILE_SIZE };

struct Coords
{
    size_t x;
    size_t y;

    Coords() = delete;

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
