#include "entities.hpp"

#include "seblib.hpp"
#include "settings.hpp"

#include <cassert>
#include <ranges>
#include <span>
#include <utility>

namespace sl = seblib;

unsigned Entities::spawn(const Entity type)
{
    if (type == Entity::Player)
    {
        assert(m_entities[PLAYER_ID] == std::nullopt);

        m_entities[PLAYER_ID] = Entity::Player;
        m_entity_ids[type].push_back(PLAYER_ID);

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

// not marked const to allow creating vector for key if it doesn't exist already
std::vector<unsigned> const& Entities::entity_ids(Entity entity)
{
    return m_entity_ids[entity];
}

std::vector<unsigned> const& Entities::to_destroy() const
{
    return m_to_destroy;
}

void Entities::destroy_entity(const unsigned id)
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

void Entities::clear_to_destroy()
{
    m_to_destroy.clear();
}

namespace entities
{
AttackDetails attack_details(const Attack attack)
{
    switch (attack)
    { // NOLINTBEGIN(*magic-numbers)
    case Attack::Melee:
        return {
            .details = MeleeDetails{ RVector2(18.0, 7.0) },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 34,
        };
    case Attack::Projectile:
        return {
            .details = ProjectileDetails{ 500.0 },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 25,
        };
    case Attack::Sector:
        return {
            .details = SectorDetails{ .radius = 50.0,
                                      .ang = sl::math::degrees_to_radians(40.0),
                                      .internal_offset = 20.0, 
                                      .external_offset = 15.0, },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 25,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace entities
