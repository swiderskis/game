#ifndef TILES_HPP_
#define TILES_HPP_

#include "seblib.hpp"

#include <cstdint>

inline constexpr float TILE_LEN{ 16.0 };

inline constexpr seblib::SimpleVec2 TILE_SIZE{ TILE_LEN, TILE_LEN };

enum class Tile : uint8_t
{
    None = 0,

    Brick,
};

#endif
