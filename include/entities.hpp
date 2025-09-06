#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstdint>
#include <variant>

enum class Entity : uint8_t
{
    None = 0,

    Player,
    Projectile,
    Enemy,
    Melee,
    Sector,
    DamageLine,
};

enum class Attack : uint8_t
{
    Melee,
    Projectile,
    Sector,
};

struct MeleeDetails
{
    raylib::Vector2 size;
};

struct ProjectileDetails
{
    float speed;
};

struct SectorDetails
{
    float radius;
    float angle;
    float line_offset;   // offset of lines from sector origin point
    float sector_offset; // offset from parent entity
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

namespace entities
{
auto attack_details(Attack attack) -> AttackDetails;
} // namespace entities

#endif
