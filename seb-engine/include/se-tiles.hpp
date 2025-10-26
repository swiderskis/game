#ifndef SE_TILES_HPP_
#define SE_TILES_HPP_

#include "se-bbox.hpp"
#include "se-sprite.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"
#include "sl-log.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

namespace seb_engine
{
namespace ranges = std::ranges;
namespace views = std::views;
namespace rl = raylib;
namespace sl = seblib;

enum class TileType : uint8_t
{
    Empty = 0,

    Block,
};

template <sl::Enumerable Sprite>
struct TileDetails
{
    TileType type;
    Sprite sprite;
};

// register tile details by specialising this template
template <sl::Enumerable Tile, sl::Enumerable Sprite>
struct TileDetailsLookup
{
    static auto get(Tile) -> TileDetails<Sprite>;
};

// assumes Tile has a "no tile" value of 0
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
class World
{
public:
    auto place_tile(Tile tile, Coords<TileSize> coords) -> void;
    auto replace_tile(Tile tile, Coords<TileSize> coords) -> void;
    auto remove_tile(Coords<TileSize> coords) -> void;
    auto draw(rl::Texture const& texture_sheet, float dt) -> void;
    [[nodiscard]] auto tiles() const -> std::vector<Tile> const&;
    [[nodiscard]] auto cboxes() const -> std::vector<rl::Rectangle> const&;
    auto draw_cboxes() const -> void;
    auto calculate_cboxes() -> void;
    [[nodiscard]] auto row(size_t y, size_t min_x, size_t max_x) const;
    [[nodiscard]] auto col(size_t x, size_t min_y, size_t max_y) const;
    [[nodiscard]] auto tile_cbox(Coords<TileSize> coords) const -> BBoxVariant;
    [[nodiscard]] auto new_tile_cbox(Tile tile, Coords<TileSize> coords) const -> BBoxVariant;
    [[nodiscard]] auto at(Coords<TileSize> coords) const -> Tile;

private:
    std::vector<Tile> m_tiles{ Width * Height, static_cast<Tile>(0) };
    std::vector<rl::Rectangle> m_cboxes;
    Sprites<Width * Height, Sprite> m_sprites;

    static TileDetailsLookup<Tile, Sprite> s_details;

    [[nodiscard]] auto at(size_t id) const -> Tile;
    [[nodiscard]] auto at_mut(Coords<TileSize> coords) -> Tile&;
    [[nodiscard]] auto at_mut(size_t id) -> Tile&;
    [[nodiscard]] auto coords_from_id(size_t id) const -> Coords<TileSize>;
    [[nodiscard]] auto id_from_coords(Coords<TileSize> coords) const -> size_t;
    [[nodiscard]] auto tile_in_cboxes(Coords<TileSize> coords) const -> bool;
    [[nodiscard]] auto cbox_from_tile_type(TileType type) const -> BBox;
};

} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::place_tile(const Tile tile, const Coords<TileSize> coords) -> void
{
    if (at(coords) == static_cast<Tile>(0))
    {
        replace_tile(tile, coords);
    }
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::replace_tile(const Tile tile, const Coords<TileSize> coords) -> void
{
    if (coords.x > Width || coords.y > Height)
    {
        return;
    }

    const auto sprite{ s_details.get(tile).sprite };
    at_mut(coords) = tile;
    m_sprites.set(id_from_coords(coords), sprite);
    calculate_cboxes();
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::remove_tile(const Coords<TileSize> coords) -> void
{
    replace_tile(static_cast<Tile>(0), coords);
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::draw(rl::Texture const& texture_sheet, const float dt) -> void
{
    for (const auto [id, tile] : m_tiles | views::enumerate)
    {
        m_sprites.draw(texture_sheet, coords_from_id(id), id, dt, false);
    }
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::tiles() const -> std::vector<Tile> const&
{
    return m_tiles;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::cboxes() const -> std::vector<rl::Rectangle> const&
{
    return m_cboxes;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::draw_cboxes() const -> void
{
    for (const auto cbox : m_cboxes)
    {
        cbox.DrawLines(::RED);
    }
}

// TODO account for different tile sizes, current logic assumes full tiles
template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::calculate_cboxes() -> void
{
    m_cboxes.clear();
    const auto non_empty{ [](const auto tile) { return std::get<1>(tile) != static_cast<Tile>(0); } };
    for (const auto [id, tile] : tiles() | views::enumerate | views::filter(non_empty))
    {
        const auto coords{ coords_from_id(id) };
        rl::Rectangle tile_cbox{ coords, rl::Vector2{ TileSize, TileSize } };
        const auto collisions{ m_cboxes
                               | views::transform([tile_cbox](const auto existing_cbox)
                                                  { return bbox::collides(tile_cbox, existing_cbox); }) };
        if (ranges::any_of(collisions, std::identity{}))
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
            const auto top_not_empty{ check_top
                                      && ranges::all_of(row(coords_max.y, coords.x, coords_max.x - 1), non_empty) };
            const auto top_in_cboxes{ ranges::any_of(
                views::iota(coords.x, coords_max.x)
                    | views::transform([coords_max, this](const auto x)
                                       { return tile_in_cboxes({ x, coords_max.y }); }),
                std::identity{}
            ) };
            const auto right_not_empty{ check_right
                                        && ranges::all_of(col(coords_max.x, coords.y, coords_max.y - 1), non_empty) };
            if (top_right_not_empty && top_not_empty && right_not_empty && !top_in_cboxes)
            {
                coords_max += { 1, 1 };
                tile_cbox.y -= TileSize;
                tile_cbox.width += TileSize;
                tile_cbox.height += TileSize;
            }
            else if (top_not_empty && !top_in_cboxes)
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

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::row(const size_t y, const size_t min_x, const size_t max_x) const
{
    return m_tiles
        | views::enumerate
        | views::filter(
               [this, y, min_x, max_x](const auto tile)
               {
                   const auto coords{ coords_from_id(std::get<0>(tile)) };

                   return coords.y == y && coords.x >= min_x && coords.x <= max_x;
               }
        );
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::col(const size_t x, const size_t min_y, const size_t max_y) const
{
    return m_tiles
        | views::enumerate
        | views::filter(
               [this, x, min_y, max_y](const auto tile)
               {
                   const auto coords{ coords_from_id(std::get<0>(tile)) };

                   return coords.x == x && coords.y >= min_y && coords.y <= max_y;
               }
        );
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::tile_cbox(const Coords<TileSize> coords) const -> BBoxVariant
{
    const auto tile{ at(coords) };

    return cbox_from_tile_type(s_details.get(tile).type).val(coords);
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::new_tile_cbox(Tile tile, Coords<TileSize> coords) const
    -> BBoxVariant
{
    return cbox_from_tile_type(s_details.get(tile).type).val(coords);
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at(const Coords<TileSize> coords) const -> Tile
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    const auto tile{ m_tiles[coords.y + (Height * coords.x)] };
    slog::log(slog::TRC, "Tile at {} is {}", coords, std::to_underlying(tile));

    return tile;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at(const size_t id) const -> Tile
{
    return at(coords_from_id(id));
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at_mut(const Coords<TileSize> coords) -> Tile&
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::at_mut(const size_t id) -> Tile&
{
    return at_mut(coords_from_id(id));
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::coords_from_id(const size_t id) const -> Coords<TileSize>
{
    return Coords<TileSize>{ id / Height, id % Height };
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::id_from_coords(const Coords<TileSize> coords) const -> size_t
{
    return (coords.x * Height) + coords.y;
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::tile_in_cboxes(Coords<TileSize> coords) const -> bool
{
    const auto cbox_tile{ tile_cbox(coords) };

    return ranges::any_of(
        m_cboxes | views::transform([cbox_tile](const auto cbox) { return bbox::collides(cbox, cbox_tile); }),
        std::identity{}
    );
}

template <sl::Enumerable Tile, sl::Enumerable Sprite, size_t Width, size_t Height, unsigned TileSize>
auto World<Tile, Sprite, Width, Height, TileSize>::cbox_from_tile_type(const TileType type) const -> BBox
{
    switch (type)
    {
    case TileType::Empty:
        return {};
    case TileType::Block:
        return BBox{ BBoxRect{ TileSize, TileSize } };
    }

    std::unreachable();
}
} // namespace seb_engine

#endif
