#ifndef SE_ECS_HPP_
#define SE_ECS_HPP_

#include "se-sprite.hpp"
#include "seb-engine.hpp"
#include "seblib.hpp"
#include "sl-log.hpp"
#include "sl-math.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;
namespace sl = seblib;

// assumes Entity has a "no entity" value of 0
template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
class Entities
{
public:
    [[nodiscard]] auto spawn(Entity type) -> size_t;
    auto queue_destroy(size_t id) -> void;
    [[nodiscard]] auto entities() const -> std::vector<Entity> const&;
    [[nodiscard]] auto entity_ids(Entity entity) -> std::vector<size_t> const&;
    [[nodiscard]] auto to_destroy() const -> std::vector<size_t> const&;
    auto destroy_entity(size_t id) -> void;
    auto clear_to_destroy() -> void;

private:
    std::vector<Entity> m_entities{ MaxEntities, static_cast<Entity>(0) };
    std::unordered_map<Entity, std::vector<size_t>> m_entity_ids;
    std::vector<size_t> m_to_destroy;
};

class IComp
{
public:
    IComp(const IComp&) = default;
    IComp(IComp&&) = default;
    virtual ~IComp() = default;

    virtual auto reset(size_t id) -> void = 0;

    auto operator=(const IComp&) -> IComp& = default;
    auto operator=(IComp&&) -> IComp& = default;

protected:
    IComp() = default;
};

template <size_t MaxEntities, typename Comp>
class Component : public IComp
{
public:
    auto reset(size_t id) -> void override;
    auto vec() -> std::vector<Comp>&;

private:
    std::vector<Comp> m_vec{ MaxEntities, Comp{} };
};

template <size_t MaxEntities>
class EntityComponents;

template <size_t MaxEntities>
class Components
{
public:
    Components();

    auto uninit_destroyed_entity(size_t id) -> void;
    [[nodiscard]] auto by_id(size_t id) -> EntityComponents<MaxEntities>;
    template <typename Comp>
    [[maybe_unused]] auto reg() -> Component<MaxEntities, Comp>*;
    template <typename Comp>
    [[nodiscard]] auto vec() -> std::vector<Comp>&;
    template <typename Comp>
    [[nodiscard]] auto get(size_t id) -> Comp&;
    auto move(float dt) -> void;

    friend class EntityComponents<MaxEntities>;

private:
    std::unordered_map<size_t, std::unique_ptr<IComp>> m_components;

    template <typename Comp>
    auto component() -> Component<MaxEntities, Comp>*;
};

template <size_t MaxEntities>
class EntityComponents
{
public:
    EntityComponents() = delete;

    template <typename Comp>
    [[nodiscard]] auto get() -> Comp&;

    friend class Components<MaxEntities>;

private:
    Components<MaxEntities>* m_components;
    size_t m_id;

    EntityComponents(Components<MaxEntities>& components, size_t id);
};

using BBoxVariant = std::variant<rl::Rectangle, sm::Circle, sm::Line>;

class BBox
{
public:
    BBox() = default;
    explicit BBox(BBoxVariant bbox);
    explicit BBox(BBoxVariant bbox, rl::Vector2 offset);

    auto sync(rl::Vector2 pos) -> void;
    [[nodiscard]] auto collides(BBox other_bbox) const -> bool;
    [[nodiscard]] auto x_overlaps(BBox other_bbox) const -> bool;
    [[nodiscard]] auto y_overlaps(BBox other_bbox) const -> bool;
    [[nodiscard]] auto val() const -> BBoxVariant;

    enum Variant : uint8_t
    {
        RECTANGLE = 0,
        CIRCLE = 1,
        LINE = 2,
    };

private:
    BBoxVariant m_bbox;
    rl::Vector2 m_offset;
};

struct TileDetails
{
    rl::Vector2 cbox_size;
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
    [[nodiscard]] auto cbox(size_t id) const -> BBox;
};

struct Position;
using Pos = seblib::Point<seb_engine::Position>;

struct Velocity;
using Vel = seblib::Point<seb_engine::Velocity>;
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
namespace slog = seblib::log;

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::spawn(const Entity type) -> size_t
{
    size_t entity_id{ 0 };
    for (const auto [id, entity] : m_entities | std::views::enumerate)
    {
        if (entity != static_cast<Entity>(0))
        {
            continue;
        }

        entity = type;
        entity_id = id;
        m_entity_ids[type].push_back(entity_id);
        break;
    }

    slog::log(slog::TRC, "Spawning entity type {} with id {}", static_cast<int>(type), entity_id);
    if (entity_id == MaxEntities)
    {
        slog::log(slog::WRN, "Maximum entities reached");
    }

    return entity_id;
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::queue_destroy(const size_t id) -> void
{
    m_to_destroy.push_back(id);
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::entities() const -> std::vector<Entity> const&
{
    return m_entities;
}

// not marked const to allow creating vector for key if it doesn't exist already
template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::entity_ids(const Entity entity) -> std::vector<size_t> const&
{
    return m_entity_ids[entity];
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::to_destroy() const -> std::vector<size_t> const&
{
    return m_to_destroy;
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::destroy_entity(const size_t id) -> void
{
    auto& entity{ m_entities[id] };
    // possible for an entity to be queued for destruction multiple times
    if (entity == static_cast<Entity>(0))
    {
        return;
    }

    slog::log(slog::TRC, "Destroying entity type {} with id {}", static_cast<int>(entity), id);
    auto& entity_ids{ m_entity_ids[entity] };
    entity_ids.erase(std::ranges::find(entity_ids, id));
    entity = static_cast<Entity>(0);
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::clear_to_destroy() -> void
{
    m_to_destroy.clear();
}

template <size_t MaxEntities, typename Comp>
auto Component<MaxEntities, Comp>::reset(const size_t id) -> void
{
    m_vec[id] = Comp{};
}

template <size_t MaxEntities, typename Comp>
auto Component<MaxEntities, Comp>::vec() -> std::vector<Comp>&
{
    return m_vec;
}

template <size_t MaxEntities>
Components<MaxEntities>::Components()
{
    reg<Pos>();
    reg<Vel>();
    reg<BBox>();
}

template <size_t MaxEntities>
auto Components<MaxEntities>::uninit_destroyed_entity(const size_t id) -> void
{
    for (auto& [_, component] : m_components)
    {
        component->reset(id);
    }
}

template <size_t MaxEntities>
auto Components<MaxEntities>::by_id(const size_t id) -> EntityComponents<MaxEntities>
{
    return { *this, id };
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::reg() -> Component<MaxEntities, Comp>*
{
    m_components.emplace(typeid(Comp).hash_code(), std::make_unique<Component<MaxEntities, Comp>>());

    return component<Comp>();
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::vec() -> std::vector<Comp>&
{
    return component<Comp>()->vec();
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::get(const size_t id) -> Comp&
{
    return component<Comp>()->vec()[id];
}

template <size_t MaxEntities>
auto Components<MaxEntities>::move(const float dt) -> void
{
    auto& pos = vec<Pos>();
    auto& vel = vec<Vel>();
    std::ranges::transform(pos, vel, pos.begin(), [dt](const auto pos, const auto vel) { return pos + (vel * dt); });
}

template <size_t MaxEntities>
template <typename Comp>
auto Components<MaxEntities>::component() -> Component<MaxEntities, Comp>*
{
#ifndef NDEBUG
    if (!m_components.contains(typeid(Comp).hash_code()))
    {
        slog::log(slog::FTL, "Components map does not contain component {}", typeid(Comp).name());
    }
#endif

    return dynamic_cast<Component<MaxEntities, Comp>*>(m_components[typeid(Comp).hash_code()].get());
}

template <size_t MaxEntities>
template <typename Comp>
auto EntityComponents<MaxEntities>::get() -> Comp&
{
    return m_components->template component<Comp>()->vec()[m_id];
}

template <size_t MaxEntities>
EntityComponents<MaxEntities>::EntityComponents(Components<MaxEntities>& components, const size_t id) :
    m_components(&components), m_id(id)
{
}

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
            rl::Rectangle{ coords_from_id(id), s_details.get(tile).cbox_size }.DrawLines(::RED);
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
auto World<TileEnum, SpriteEnum, Width, Height>::cbox(const size_t id) const -> BBox
{
    return BBox{ rl::Rectangle{ coords_from_id(id), s_details.get(at(id)).cbox_size } };
}
} // namespace seb_engine

#endif
