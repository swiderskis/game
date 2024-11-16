#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include <optional>

enum class EntityType {
    Player,
    Tile,
    Projectile
};

enum class Tile {
    Brick
};

class Entity
{
    unsigned m_id;
    std::optional<EntityType> m_type;

    explicit Entity(unsigned id);

    friend class EntityManager;

public:
    Entity() = delete;

    [[nodiscard]] unsigned id() const;
    [[nodiscard]] std::optional<EntityType> type() const;
    void clear_type();
};

#endif
