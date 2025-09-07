#ifndef SE_ENTITIES_HPP_
#define SE_ENTITIES_HPP_

#include "seblib.hpp"
#include "sl-log.hpp"

#include <cstddef>
#include <ranges>
#include <unordered_map>

namespace seb_engine
{
namespace sl = seblib;

// assumes Entity has a "no entity" value of 0
template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
class Entities
{
public:
    [[nodiscard]] auto spawn(Entity type) -> size_t;
    [[nodiscard]] auto vec() const -> std::vector<Entity> const&;
    [[nodiscard]] auto ids(Entity entity) -> std::vector<size_t> const&;
    auto destroy(size_t id) -> void;

private:
    std::vector<Entity> m_entities{ MaxEntities, static_cast<Entity>(0) };
    std::unordered_map<Entity, std::vector<size_t>> m_entity_ids;
};
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
auto Entities<MaxEntities, Entity>::vec() const -> std::vector<Entity> const&
{
    return m_entities;
}

// not marked const to allow creating vector for key if it doesn't exist already
template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::ids(const Entity entity) -> std::vector<size_t> const&
{
    return m_entity_ids[entity];
}

template <size_t MaxEntities, typename Entity>
    requires sl::Enumerable<Entity>
auto Entities<MaxEntities, Entity>::destroy(const size_t id) -> void
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
} // namespace seb_engine

#endif
