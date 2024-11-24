#include "entities.hpp"

#include <cassert>
#include <ranges>
#include <span>

unsigned EntityManager::spawn_entity(const Entity type)
{
    if (type == Entity::Player) {
        assert(entities[PLAYER_ID] == std::nullopt);

        entities[PLAYER_ID] = Entity::Player;

        return PLAYER_ID;
    }

    assert(type != Entity::Player);

    unsigned entity_id = 0;
    for (const auto [id, entity] : std::span(entities).subspan(1) | std::views::enumerate) {
        if (entity != std::nullopt) {
            continue;
        }

        entity = type;
        entity_id = id + 1; // enumerate will start at 0, subspan starts loop from m_entitites[1]
        entity_ids[type].push_back(entity_id);
        break;
    }

    assert(entity_id != PLAYER_ID);

    return entity_id;
}

void EntityManager::queue_destroy_entity(const unsigned id)
{
    entities_to_destroy.push_back(id);
}
