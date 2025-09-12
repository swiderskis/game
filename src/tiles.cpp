#include "tiles.hpp"

#include "se-bbox.hpp"
#include "se-tiles.hpp"

namespace se = seb_engine;

template <>
auto se::TileDetailsLookup<Tile>::get(const Tile tile) -> se::TileDetails
{
    switch (tile)
    { // NOLINTBEGIN(*magic-numbers)
    case Tile::None:
        return {
            .cbox = { 0.0, 0.0 },
        };
    case Tile::Brick:
        return {
            .cbox = se::BBoxRect{ TILE_SIZE },
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
