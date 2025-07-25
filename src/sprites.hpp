#ifndef SPRITE_HPP_
#define SPRITE_HPP_

#include "entities.hpp"
#include "seb-engine-sprite.hpp"

#include <cstdint>

enum class SpriteBase : int8_t
{
    None = -1,

    PlayerIdle,
    Projectile,
    EnemyDuck,
};

enum class SpriteHead : int8_t
{
    None = -1,

    PlayerIdle,
};

enum class SpriteArms : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerJump,
    PlayerAttack,
};

enum class SpriteLegs : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerWalk,
    PlayerJump,
};

enum class SpriteExtra : int8_t
{
    None = -1,

    PlayerScarfWalk,
    PlayerScarfFall,
};

enum class SpriteTile : int8_t
{
    None = -1,

    TileBrick,
};

using Sprites = seb_engine::Sprites<SpriteBase, SpriteHead, SpriteArms, SpriteLegs, SpriteExtra>;

namespace sprites
{
void lookup_set_movement_sprites(Sprites& sprites, unsigned id, Entity entity, raylib::Vector2 vel);
[[nodiscard]] float alternate_frame_y_offset(SpriteLegs legs);
} // namespace sprites

#endif
