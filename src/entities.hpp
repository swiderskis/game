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

class Entity
{
    unsigned m_id;
    std::optional<EntityType> m_type;

    explicit Entity(const unsigned id);

    friend class EntityManager;

public:
    Entity() = delete;

    [[nodiscard]] unsigned id() const;
    [[nodiscard]] std::optional<EntityType> type() const;
    void clear_type();
};

#endif
