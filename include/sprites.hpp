#ifndef SPRITE_HPP_
#define SPRITE_HPP_

#include "entities.hpp"
#include "se-sprite.hpp"
#include "seblib.hpp"
#include "settings.hpp"

#include <cstdint>

inline constexpr float SPRITE_LEN{ 32.0 };

inline constexpr seblib::SimpleVec2 SPRITE_SIZE{ SPRITE_LEN, SPRITE_LEN };

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

using Sprites = seb_engine::Sprites<MAX_ENTITIES, SpriteBase, SpriteHead, SpriteArms, SpriteLegs, SpriteExtra>;

namespace sprites
{
auto lookup_set_movement_sprites(Sprites& sprites, size_t id, Entity entity, raylib::Vector2 vel) -> void;
[[nodiscard]] auto alternate_frame_y_offset(SpriteLegs legs) -> float;
[[nodiscard]] auto flipped_x_offset(raylib::Vector2 sprite_size) -> float;
} // namespace sprites

#endif
