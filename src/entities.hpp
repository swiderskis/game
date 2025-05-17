#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstdint>
#include <variant>

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
    raylib::Vector2 size;
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

namespace entities
{
AttackDetails attack_details(Attack attack);
} // namespace entities

#endif
