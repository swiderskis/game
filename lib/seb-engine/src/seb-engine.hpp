#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "seblib.hpp"

#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>

inline constexpr unsigned MAX_ENTITIES = 1024;

namespace seb_engine
{
template <typename Entity>
class Entities
{
    std::vector<std::optional<Entity>> m_entities{ MAX_ENTITIES, std::nullopt };
    std::unordered_map<Entity, std::vector<unsigned>> m_entity_ids;
    std::vector<unsigned> m_to_destroy;

public:
    [[nodiscard]] unsigned spawn(Entity type);
    void queue_destroy(unsigned id);
    [[nodiscard]] std::vector<std::optional<Entity>> const& entities() const;
    [[nodiscard]] std::vector<unsigned> const& entity_ids(Entity entity);
    [[nodiscard]] std::vector<unsigned> const& to_destroy() const;
    void destroy_entity(unsigned id);
    void clear_to_destroy();
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

template <typename Entity>
unsigned Entities<Entity>::spawn(const Entity type)
{
    unsigned entity_id = 0;
    for (const auto [id, entity] : m_entities | std::views::enumerate)
    {
        if (entity != std::nullopt)
        {
            continue;
        }

        entity = type;
        entity_id = id;
        m_entity_ids[type].push_back(entity_id);
        break;
    }

    slog::log(slog::INF, "Spawning entity type {} with id {}", (int)type, entity_id);

    return entity_id;
}

template <typename Entity>
void Entities<Entity>::queue_destroy(const unsigned id)
{
    m_to_destroy.push_back(id);
}

template <typename Entity>
std::vector<std::optional<Entity>> const& Entities<Entity>::entities() const
{
    return m_entities;
}

// not marked const to allow creating vector for key if it doesn't exist already
template <typename Entity>
std::vector<unsigned> const& Entities<Entity>::entity_ids(Entity entity)
{
    return m_entity_ids[entity];
}

template <typename Entity>
std::vector<unsigned> const& Entities<Entity>::to_destroy() const
{
    return m_to_destroy;
}

template <typename Entity>
void Entities<Entity>::destroy_entity(const unsigned id)
{
    auto& entity = m_entities[id];
    // possible for an entity to be queued for destruction multiple times, leads to already being nullopt
    if (entity == std::nullopt)
    {
        return;
    }

    auto& entity_ids = m_entity_ids[entity.value()];
    entity_ids.erase(std::ranges::find(entity_ids, id));
    entity = std::nullopt;
}

template <typename Entity>
void Entities<Entity>::clear_to_destroy()
{
    m_to_destroy.clear();
}
} // namespace seb_engine

#endif
