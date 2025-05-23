#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "settings.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

enum class Entity : uint8_t
{
    Player,
    Tile,
    Projectile,
    Enemy,
    Melee,
    Sector,
    DamageLine,
};

enum class Tile : uint8_t
{
    Brick,
};

enum class Attack : uint8_t
{
    Melee,
    Projectile,
    Sector,
};

struct MeleeDetails
{
    RVector2 size;
};

struct ProjectileDetails
{
    float speed;
};

struct SectorDetails
{
    float radius;
    float ang;
    float internal_offset; // offset from sector origin point
    float external_offset; // offset from parent entity
};

struct AttackDetails
{
    std::variant<MeleeDetails, ProjectileDetails, SectorDetails> details;
    float lifespan;
    float delay;
    float cooldown;
    int damage;
};

enum class Enemy : uint8_t
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

namespace entities
{
AttackDetails attack_details(Attack attack);
} // namespace entities

#endif
