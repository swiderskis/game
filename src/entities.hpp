#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include "settings.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

enum class Entity
{
    Player,
    Tile,
    Projectile,
    Enemy,
    Melee,
};

enum class Tile
{
    Brick,
};

enum class Attack
{
    Melee,
    Projectile,
};

enum class Enemy
{
    Duck,
};

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

#endif
