#ifndef SE_TILES_HPP_
#define SE_TILES_HPP_

#include "se-bbox.hpp"
#include "se-sprite.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"
#include "sl-log.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;

enum class TileType : uint8_t
{
    Empty = 0,

    Block,
};

struct TileDetails
{
    TileType type;
};

// register tile details by specialising this template
template <sl::Enumerable Tile>
struct TileDetailsLookup
{
    static auto get(Tile) -> TileDetails;
};

// assumes Tile has a "no tile" value of 0
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
class World
{
public:
    static TileDetailsLookup<Tile> s_details;

    auto spawn(Coords<TileSize> coords, Tile tile, Sprite sprite) -> void;
    auto draw(rl::Texture const& texture_sheet, float dt) -> void;
    [[nodiscard]] auto tiles() const -> std::vector<Tile> const&;
    [[nodiscard]] auto cboxes() const -> std::vector<rl::Rectangle> const&;
    auto draw_cboxes() const -> void;
    auto calculate_cboxes() -> void;

private:
    std::vector<Tile> m_tiles{ Width * Height, static_cast<Tile>(0) };
    std::vector<rl::Rectangle> m_cboxes;
    Sprites<Width * Height, Sprite> m_sprites;

    [[nodiscard]] auto at(Coords<TileSize> coords) const -> Tile;
    [[nodiscard]] auto at(size_t id) const -> Tile;
    [[nodiscard]] auto at_mut(Coords<TileSize> coords) -> Tile&;
    [[nodiscard]] auto at_mut(size_t id) -> Tile&;
    [[nodiscard]] auto coords_from_id(size_t id) const -> Coords<TileSize>;
    [[nodiscard]] auto id_from_coords(Coords<TileSize> coords) const -> size_t;
    [[nodiscard]] auto cbox(size_t id) const -> BBoxVariant;
    [[nodiscard]] auto row(size_t y, size_t min_x, size_t max_x) const;
    [[nodiscard]] auto col(size_t x, size_t min_y, size_t max_y) const;
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::spawn(
    const Coords<TileSize> coords, const Tile tile, const Sprite sprite
) -> void
{
    at_mut(coords) = tile;
    m_sprites.set(id_from_coords(coords), sprite);
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::draw(rl::Texture const& texture_sheet, const float dt) -> void
{
    for (const auto [id, tile] : m_tiles | std::views::enumerate)
    {
        m_sprites.draw(texture_sheet, coords_from_id(id), id, dt, false);
    }
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::tiles() const -> std::vector<Tile> const&
{
    return m_tiles;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::cboxes() const -> std::vector<rl::Rectangle> const&
{
    return m_cboxes;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::draw_cboxes() const -> void
{
    for (const auto cbox : m_cboxes)
    {
        cbox.DrawLines(::RED);
    }
}

// TODO account for different tile sizes, current logic assumes full tiles
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::calculate_cboxes() -> void
{
    m_cboxes.clear();
    const auto non_empty{ [](const auto tile) -> bool { return std::get<1>(tile) != static_cast<Tile>(0); } };
    for (const auto [id, tile] : tiles() | std::views::enumerate | std::views::filter(non_empty))
    {
        const auto coords{ coords_from_id(id) };
        rl::Rectangle tile_cbox{ coords, rl::Vector2{ TileSize, TileSize } };
        const auto collisions{ m_cboxes
                               | std::views::transform(
                                   [tile_cbox](const auto existing_cbox) -> bool
                                   { return bbox::collides(tile_cbox, existing_cbox); }
                               ) };
        if (std::ranges::any_of(collisions, [](const auto collides) -> bool { return collides; }))
        {
            slog::log(slog::TRC, "Skipping tile at ({}, {})", coords.x, coords.y);
            continue;
        }

        auto check_right{ true };
        auto check_top{ true };
        auto coords_max{ coords + Coords<TileSize>{ 1, 1 } };
        while (check_right || check_top)
        {
            const auto top_right_not_empty{ check_right && check_top && at(coords_max) != static_cast<Tile>(0) };
            const auto top_not_empty{
                check_top && std::ranges::all_of(row(coords_max.y, coords.x, coords_max.x - 1), non_empty)
            };
            const auto right_not_empty{
                check_right && std::ranges::all_of(col(coords_max.x, coords.y, coords_max.y - 1), non_empty)
            };
            if (top_right_not_empty && top_not_empty && right_not_empty)
            {
                coords_max += { 1, 1 };
                tile_cbox.y -= TileSize;
                tile_cbox.width += TileSize;
                tile_cbox.height += TileSize;
            }
            else if (top_not_empty)
            {
                coords_max += { 0, 1 };
                check_right = false;
                tile_cbox.y -= TileSize;
                tile_cbox.height += TileSize;
            }
            else if (right_not_empty)
            {
                coords_max += { 1, 0 };
                check_top = false;
                tile_cbox.width += TileSize;
            }
            else
            {
                break;
            }
        }

        m_cboxes.emplace_back(tile_cbox);
    }
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at(const Coords<TileSize> coords) const -> Tile
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at(const size_t id) const -> Tile
{
    return at(coords_from_id(id));
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at_mut(const Coords<TileSize> coords) -> Tile&
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at_mut(const size_t id) -> Tile&
{
    return at_mut(coords_from_id(id));
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::coords_from_id(const size_t id) const -> Coords<TileSize>
{
    return Coords<TileSize>{ id / Height, id % Height };
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::id_from_coords(const Coords<TileSize> coords) const -> size_t
{
    return (coords.x * Height) + coords.y;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::cbox(const size_t id) const -> BBoxVariant
{
    return BBox{ s_details.get(at(id)).cbox }.val(coords_from_id(id));
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::row(const size_t y, const size_t min_x, const size_t max_x) const
{
    return m_tiles
        | std::views::enumerate
        | std::views::filter(
               [this, y, min_x, max_x](const auto tile) -> auto
               {
                   const auto coords{ coords_from_id(std::get<0>(tile)) };

                   return coords.y == y && coords.x >= min_x && coords.x <= max_x;
               }
        );
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, float TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::col(const size_t x, const size_t min_y, const size_t max_y) const
{
    return m_tiles
        | std::views::enumerate
        | std::views::filter(
               [this, x, min_y, max_y](const auto tile) -> auto
               {
                   const auto coords{ coords_from_id(std::get<0>(tile)) };

                   return coords.x == x && coords.y >= min_y && coords.y <= max_y;
               }
        );
}
} // namespace seb_engine

#endif
