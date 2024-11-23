#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include "settings.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

enum class Entity {
    Player,
    Tile,
    Projectile,
    Enemy
};

enum class Tile {
    Brick
};

class EntityManager
{
    std::vector<std::optional<Entity>> m_entities{ MAX_ENTITIES, std::nullopt };
    std::unordered_map<Entity, std::vector<unsigned>> m_entity_ids;
    std::vector<unsigned> m_entities_to_destroy;

    [[nodiscard]] unsigned spawn_entity(Entity type);
    void queue_destroy_entity(unsigned id);

    friend class Game;
};

#endif
