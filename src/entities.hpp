#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include <optional>

enum class EntityType {
    Player,
    Tile,
    Projectile,
    Enemy
};

enum class Tile {
    Brick
};

struct Entity {
    std::optional<EntityType> type;

    Entity() = delete;

    [[nodiscard]] unsigned id() const;

private:
    unsigned m_id;

    explicit Entity(unsigned id);

    friend class EntityManager;
};

#endif
