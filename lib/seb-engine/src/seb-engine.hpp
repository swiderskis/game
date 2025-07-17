#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "seblib.hpp"

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;

inline constexpr unsigned MAX_ENTITIES = 1024;
inline constexpr unsigned MAX_TILES = 1024;

inline constexpr float TILE_SIZE = 16.0;

struct Coord
{
    sl::SimpleVec2 pos;

    Coord() = delete;

    constexpr Coord(int x, int y);

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
constexpr Coord::Coord(int x, int y) : pos(static_cast<float>(x) * TILE_SIZE, static_cast<float>(-y) * TILE_SIZE)
{
}
} // namespace seb_engine

#endif
