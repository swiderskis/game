#include "tiles.hpp"

#include "entities.hpp"
#include "se-ecs.hpp"

namespace se = seb_engine;

template <>
se::TileDetails se::TileDetailsLookup<Tile>::get(Tile tile)
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
