#include "entities.hpp"

#include "components.hpp"

#include <cassert>
#include <ranges>
#include <span>
#include <utility>

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

void Entities::destroy_entity(const unsigned id)
{
    auto& entity = m_entities[id];
    if (entity == std::nullopt) // possible for an entity to be queued for destruction multiple times,
    {                           // leads to already being nullopt
        return;
    }

    auto& entity_ids = m_entity_ids[entity.value()];
    entity_ids.erase(std::ranges::find(entity_ids, id));
    entity = std::nullopt;
}

void Entities::clear_to_destroy()
{
    m_to_destroy.clear();
}

namespace entities
{
AttackDetails attack_details(const Attack attack)
{
    float duration = 0.0;
    switch (attack)
    { // NOLINTBEGIN(*magic-numbers)
    case Attack::Melee:
        duration = components::sprite_details(SpriteArms::PlayerAttack).frame_duration;
        return { MeleeDetails{ { 18.0, 7.0 }, { 24.0, 9.0 } }, duration, 0.0, 0.5 };
    case Attack::Projectile:
        return { ProjectileDetails{ 500.0 }, 0.3, 0.0, 0.5 };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace entities
