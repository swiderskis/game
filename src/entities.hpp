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

struct EntityManager {
    std::vector<std::optional<Entity>> entities{ MAX_ENTITIES, std::nullopt };
    std::unordered_map<Entity, std::vector<unsigned>> entity_ids;
    std::vector<unsigned> entities_to_destroy;

    [[nodiscard]] unsigned spawn_entity(Entity type);
    void queue_destroy_entity(unsigned id);

private:
    EntityManager() = default;

    friend class Game;
};

#endif
