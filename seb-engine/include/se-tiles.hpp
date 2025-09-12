#ifndef SE_TILES_HPP_
#define SE_TILES_HPP_

#include "se-bbox.hpp"
#include "se-sprite.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

namespace seb_engine
{
namespace rl = raylib;
namespace sl = seblib;

struct TileDetails
{
    BBoxRect cbox;
};

// register tile details by specialising this template
template <typename TileEnum>
    requires sl::Enumerable<TileEnum>
struct TileDetailsLookup
{
    static auto get(TileEnum) -> TileDetails;
};

// assumes TileEnum has a "no tile" value of 0
template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
class World
{
public:
    static TileDetailsLookup<TileEnum> s_details;

    auto spawn(Coords coords, TileEnum tile, SpriteEnum sprite) -> void;
    auto draw(rl::Texture const& texture_sheet, float dt) -> void;
    [[nodiscard]] auto tiles() const -> std::vector<TileEnum> const&;
    [[nodiscard]] auto cboxes() const;
    auto draw_cboxes() const -> void;

private:
    std::vector<TileEnum> m_tiles{ Width * Height, static_cast<TileEnum>(0) };
    Sprites<Width * Height, SpriteEnum> m_sprites;

    [[nodiscard]] auto at(Coords coords) const -> TileEnum;
    [[nodiscard]] auto at(size_t id) const -> TileEnum;
    [[nodiscard]] auto at_mut(Coords coords) -> TileEnum&;
    [[nodiscard]] auto at_mut(size_t id) -> TileEnum&;
    [[nodiscard]] auto coords_from_id(size_t id) const -> Coords;
    [[nodiscard]] auto id_from_coords(Coords coords) const -> size_t;
    [[nodiscard]] auto cbox(size_t id) const -> BBoxVariant;
};
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::spawn(const Coords coords,
                                                       const TileEnum tile,
                                                       const SpriteEnum sprite) -> void
{
    at_mut(coords) = tile;
    m_sprites.set(id_from_coords(coords), sprite);
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::draw(rl::Texture const& texture_sheet, const float dt) -> void
{
    for (const auto [id, tile] : m_tiles | std::views::enumerate)
    {
        m_sprites.draw(texture_sheet, coords_from_id(id), id, dt, false);
    }
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::tiles() const -> std::vector<TileEnum> const&
{
    return m_tiles;
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::cboxes() const
{
    return m_tiles | std::views::enumerate
           | std::views::filter([](const auto x) { return std::get<1>(x) != static_cast<TileEnum>(0); })
           | std::views::transform([this](const auto x) { return cbox(std::get<0>(x)); });
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::draw_cboxes() const -> void
{
    for (const auto [id, tile] : m_tiles | std::views::enumerate)
    {
        if (tile != static_cast<TileEnum>(0))
        {
            rl::Rectangle{ coords_from_id(id), s_details.get(tile).cbox.size }.DrawLines(::RED);
        }
    }
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::at(const Coords coords) const -> TileEnum
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::at(const size_t id) const -> TileEnum
{
    return at(coords_from_id(id));
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::at_mut(const Coords coords) -> TileEnum&
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::at_mut(const size_t id) -> TileEnum&
{
    return at_mut(coords_from_id(id));
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::coords_from_id(size_t id) const -> Coords
{
    return Coords{ id / Height, id % Height };
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::id_from_coords(Coords coords) const -> size_t
{
    return (coords.x * Height) + coords.y;
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::cbox(const size_t id) const -> BBoxVariant
{
    return BBox{ s_details.get(at(id)).cbox }.val(coords_from_id(id));
}
} // namespace seb_engine

#endif
