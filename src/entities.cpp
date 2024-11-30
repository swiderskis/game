#include "entities.hpp"

#include <cassert>
#include <ranges>
#include <span>

unsigned Entities::spawn(const Entity type)
{
    if (type == Entity::Player)
    {
        assert(m_entities[PLAYER_ID] == std::nullopt);

        m_entities[PLAYER_ID] = Entity::Player;

        return PLAYER_ID;
    }

    assert(type != Entity::Player);

    unsigned entity_id = 0;
    for (const auto [id, entity] : std::span(m_entities).subspan(1) | std::views::enumerate)
    {
        if (entity != std::nullopt)
        {
            continue;
        }

        entity = type;
        entity_id = id + 1; // enumerate will start at 0, subspan starts loop from m_entitites[1]
        m_entity_ids[type].push_back(entity_id);

        break;
    }

    assert(entity_id != PLAYER_ID);

    return entity_id;
}

void Entities::queue_destroy(const unsigned id)
{
    m_to_destroy.push_back(id);
}

std::vector<std::optional<Entity>> const& Entities::entities() const
{
    return m_entities;
}

std::vector<unsigned> const& Entities::entity_ids(Entity entity) // not marked const to allow creating vector for key
{                                                                // if it doesn't exist already
    return m_entity_ids[entity];
}

std::vector<unsigned> const& Entities::to_destroy() const
{
    return m_to_destroy;
}

void Entities::destroy_queued()
{
    for (const unsigned id : m_to_destroy)
    {
        auto& entity = m_entities[id];
        if (entity == std::nullopt) // possible for an entity to be queued for destruction multiple times,
        {                           // leads to already being nullopt
            continue;
        }

        auto& entity_ids = m_entity_ids[entity.value()];
        entity_ids.erase(std::ranges::find(entity_ids, id));
        entity = std::nullopt;
    }

    m_to_destroy.clear();
}
