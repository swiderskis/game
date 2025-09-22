#include "tiles.hpp"

#include "se-tiles.hpp"

namespace se = seb_engine;

template <>
auto se::TileDetailsLookup<Tile>::get(const Tile tile) -> se::TileDetails
{
    switch (tile)
    { // NOLINTBEGIN(*magic-numbers)
    case Tile::None:
        return {
            .type = TileType::Empty,
        };
    case Tile::Brick:
        return {
            .type = TileType::Block,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
