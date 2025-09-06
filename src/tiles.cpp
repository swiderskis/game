#include "tiles.hpp"

#include "entities.hpp"
#include "se-ecs.hpp"

namespace se = seb_engine;

template <>
auto se::TileDetailsLookup<Tile>::get(const Tile tile) -> se::TileDetails
{
    switch (tile)
    { // NOLINTBEGIN(*magic-numbers)
    case Tile::None:
        return {
            .cbox_size{ 0.0, 0.0 },
        };
    case Tile::Brick:
        return {
            .cbox_size{ TILE_SIZE },
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
