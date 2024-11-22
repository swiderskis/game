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

private:
    unsigned m_id;

    explicit Entity(unsigned id);

    friend class EntityManager;

public:
    Entity() = delete;

    [[nodiscard]] unsigned id() const;
};

#endif
