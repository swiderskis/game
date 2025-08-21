#ifndef SEB_ENGINE_ECS_HPP_
#define SEB_ENGINE_ECS_HPP_

#include "seb-engine-sprite.hpp"
#include "seb-engine.hpp"
#include "seblib-log.hpp"
#include "seblib-math.hpp"
#include "seblib.hpp"

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

// assumes Entity has a "no entity" value of -1
template <typename Entity>
    requires sl::Enumerable<Entity>
class Entities
{
    std::vector<Entity> m_entities{ MAX_ENTITIES, static_cast<Entity>(-1) };
    std::unordered_map<Entity, std::vector<unsigned>> m_entity_ids;
    std::vector<unsigned> m_to_destroy;

public:
    [[nodiscard]] unsigned spawn(Entity type);
    void queue_destroy(unsigned id);
    [[nodiscard]] std::vector<Entity> const& entities() const;
    [[nodiscard]] std::vector<unsigned> const& entity_ids(Entity entity);
    [[nodiscard]] std::vector<unsigned> const& to_destroy() const;
    void destroy_entity(unsigned id);
    void clear_to_destroy();
};

struct IComp
{
    IComp(const IComp&) = default;
    IComp(IComp&&) = default;
    IComp& operator=(const IComp&) = default;
    IComp& operator=(IComp&&) = default;
    virtual ~IComp() = default;

    virtual void reset(unsigned id) = 0;

protected:
    IComp() = default;
};

template <typename Comp>
class Component : public IComp
{
    std::vector<Comp> m_vec{ MAX_ENTITIES, Comp{} };

public:
    void reset(unsigned id) override;
    std::vector<Comp>& vec();
};

class EntityComponents;

class Components
{
    std::unordered_map<size_t, std::unique_ptr<IComp>> m_components;

    template <typename Comp>
    Component<Comp>* component();

    friend class EntityComponents;

public:
    Components();

    void uninit_destroyed_entity(unsigned id);
    [[nodiscard]] EntityComponents by_id(unsigned id);
    template <typename Comp>
    [[maybe_unused]] Component<Comp>* reg();
    template <typename Comp>
    [[nodiscard]] std::vector<Comp>& vec();
    template <typename Comp>
    [[nodiscard]] Comp& get(unsigned id);
    void move(float dt);
};

class EntityComponents
{
    Components* m_components;
    unsigned m_id;

    EntityComponents(Components& components, unsigned id);

    friend class Components;

public:
    EntityComponents() = delete;

    template <typename Comp>
    [[nodiscard]] Comp& get();
};

using BBoxVariant = std::variant<rl::Rectangle, sm::Circle, sm::Line>;

class BBox
{
    BBoxVariant m_bbox{ rl::Rectangle{} };
    rl::Vector2 m_offset{ 0.0, 0.0 };

public:
    BBox() = default;
    explicit BBox(BBoxVariant bbox, rl::Vector2 offset);

    void sync(rl::Vector2 pos);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;
    [[nodiscard]] BBoxVariant bbox() const;

    enum Variant : uint8_t
    {
        RECTANGLE = 0,
        CIRCLE = 1,
        LINE = 2,
    };
};

// assumes TileEnum has a "no tile" value of -1
template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
class World
{
    std::vector<TileEnum> m_tiles{ Width * Height, static_cast<TileEnum>(-1) };
    Sprites<SpriteEnum> m_sprites{ Width * Height };

    [[nodiscard]] TileEnum at(Coords coords) const;
    [[nodiscard]] TileEnum& at_mut(Coords coords);
    [[nodiscard]] Coords coords_from_id(size_t id) const;
    [[nodiscard]] size_t id_from_coords(Coords coords) const;
    [[nodiscard]] BBox cbox(size_t id) const;

public:
    void spawn(Coords coords, TileEnum tile, SpriteEnum sprite);
    void draw(rl::Texture const& texture_sheet, float dt);
    [[nodiscard]] std::vector<TileEnum> const& tiles() const;
    [[nodiscard]] auto cboxes() const;
    void draw_cboxes() const;
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

template <typename Entity>
    requires sl::Enumerable<Entity>
unsigned Entities<Entity>::spawn(const Entity type)
{
    unsigned entity_id = 0;
    for (const auto [id, entity] : m_entities | std::views::enumerate)
    {
        if (entity != static_cast<Entity>(-1))
        {
            continue;
        }

        entity = type;
        entity_id = id;
        m_entity_ids[type].push_back(entity_id);
        break;
    }

    slog::log(slog::TRC, "Spawning entity type {} with id {}", static_cast<int>(type), entity_id);
    if (entity_id == MAX_ENTITIES)
    {
        slog::log(slog::WRN, "Maximum entities reached");
    }

    return entity_id;
}

template <typename Entity>
    requires sl::Enumerable<Entity>
void Entities<Entity>::queue_destroy(const unsigned id)
{
    m_to_destroy.push_back(id);
}

template <typename Entity>
    requires sl::Enumerable<Entity>
std::vector<Entity> const& Entities<Entity>::entities() const
{
    return m_entities;
}

// not marked const to allow creating vector for key if it doesn't exist already
template <typename Entity>
    requires sl::Enumerable<Entity>
std::vector<unsigned> const& Entities<Entity>::entity_ids(const Entity entity)
{
    return m_entity_ids[entity];
}

template <typename Entity>
    requires sl::Enumerable<Entity>
std::vector<unsigned> const& Entities<Entity>::to_destroy() const
{
    return m_to_destroy;
}

template <typename Entity>
    requires sl::Enumerable<Entity>
void Entities<Entity>::destroy_entity(const unsigned id)
{
    auto& entity = m_entities[id];
    // possible for an entity to be queued for destruction multiple times
    if (entity == static_cast<Entity>(-1))
    {
        return;
    }

    slog::log(slog::TRC, "Destroying entity type {} with id {}", static_cast<int>(entity), id);
    auto& entity_ids = m_entity_ids[entity];
    entity_ids.erase(std::ranges::find(entity_ids, id));
    entity = static_cast<Entity>(-1);
}

template <typename Entity>
    requires sl::Enumerable<Entity>
void Entities<Entity>::clear_to_destroy()
{
    m_to_destroy.clear();
}

template <typename Comp>
void Component<Comp>::reset(const unsigned id)
{
    m_vec[id] = Comp{};
}

template <typename Comp>
std::vector<Comp>& Component<Comp>::vec()
{
    return m_vec;
}

template <typename Comp>
Component<Comp>* Components::component()
{
#ifndef NDEBUG
    if (!m_components.contains(typeid(Comp).hash_code()))
    {
        slog::log(slog::FTL, "Components map does not contain component {}", typeid(Comp).name());
    }
#endif

    return dynamic_cast<Component<Comp>*>(m_components[typeid(Comp).hash_code()].get());
}

template <typename Comp>
Component<Comp>* Components::reg()
{
    m_components.emplace(typeid(Comp).hash_code(), std::make_unique<Component<Comp>>());

    return component<Comp>();
}

template <typename Comp>
std::vector<Comp>& Components::vec()
{
    return component<Comp>()->vec();
}

template <typename Comp>
Comp& Components::get(const unsigned id)
{
    return component<Comp>()->vec()[id];
}

template <typename Comp>
Comp& EntityComponents::get()
{
    return m_components->component<Comp>()->vec()[m_id];
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
TileEnum World<TileEnum, SpriteEnum, Width, Height>::at(const Coords coords) const
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
TileEnum& World<TileEnum, SpriteEnum, Width, Height>::at_mut(const Coords coords)
{
    assert(coords.x <= Width);
    assert(coords.y <= Height);

    return m_tiles[coords.y + (Height * coords.x)];
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
Coords World<TileEnum, SpriteEnum, Width, Height>::coords_from_id(size_t id) const
{
    const size_t x = id / Height;
    const size_t y = id % Height;

    return Coords{ x, y };
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
size_t World<TileEnum, SpriteEnum, Width, Height>::id_from_coords(Coords coords) const
{
    return (coords.x * Height) + coords.y;
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
BBox World<TileEnum, SpriteEnum, Width, Height>::cbox(const size_t id) const
{
    const auto coords = coords_from_id(id);

    return BBox{ rl::Rectangle{ coords, TILE_CBOX_SIZE }, rl::Vector2{ 0.0, 0.0 } };
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
void World<TileEnum, SpriteEnum, Width, Height>::spawn(const Coords coords,
                                                       const TileEnum tile,
                                                       const SpriteEnum sprite)
{
    at_mut(coords) = tile;
    m_sprites.set(id_from_coords(coords), sprite);
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
void World<TileEnum, SpriteEnum, Width, Height>::draw(rl::Texture const& texture_sheet, const float dt)
{
    for (const auto [id, tile] : m_tiles | std::views::enumerate)
    {
        m_sprites.draw(texture_sheet, coords_from_id(id), id, dt, false);
    }
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
std::vector<TileEnum> const& World<TileEnum, SpriteEnum, Width, Height>::tiles() const
{
    return m_tiles;
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
auto World<TileEnum, SpriteEnum, Width, Height>::cboxes() const
{
    return m_tiles | std::views::enumerate
           | std::views::filter([](const auto x) { return std::get<1>(x) != static_cast<TileEnum>(-1); })
           | std::views::transform([this](const auto x) { return cbox(std::get<0>(x)); });
}

template <typename TileEnum, typename SpriteEnum, size_t Width, size_t Height>
    requires sl::Enumerable<TileEnum> && sl::Enumerable<SpriteEnum>
void World<TileEnum, SpriteEnum, Width, Height>::draw_cboxes() const
{
    for (const auto [id, tile] : m_tiles | std::views::enumerate)
    {
        if (tile != static_cast<TileEnum>(-1))
        {
            const auto coords = coords_from_id(id);
            rl::Rectangle{ coords, TILE_CBOX_SIZE }.DrawLines(::RED);
        }
    }
}
} // namespace seb_engine

#endif
