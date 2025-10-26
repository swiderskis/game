#include "tiles.hpp"

#include "se-tiles.hpp"
#include "sprites.hpp"

namespace se = seb_engine;

template <>
auto se::TileDetailsLookup<Tile, SpriteTile>::get(const Tile tile) -> se::TileDetails<SpriteTile>
{
    switch (tile)
    { // NOLINTBEGIN(*magic-numbers)
    case Tile::None:
        return {
            .type = TileType::Empty,
            .sprite = SpriteTile::None,
        };
    case Tile::Brick:
        return {
            .type = TileType::Block,
            .sprite = SpriteTile::Brick,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
