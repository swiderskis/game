#ifndef SPRITE_HPP_
#define SPRITE_HPP_

#include "entities.hpp"
#include "se-sprite.hpp"
#include "settings.hpp"
#include "sl-math.hpp"

#include <cstdint>
#include <type_traits>

inline constexpr float SPRITE_LEN{ 32.0 };

inline constexpr seblib::math::Vec2 SPRITE_SIZE{ SPRITE_LEN, SPRITE_LEN };

enum class SpriteBase : uint8_t
{
    None = 0,

    PlayerIdle,
    Projectile,
    EnemyDuck,
};

enum class SpriteHead : uint8_t
{
    None = 0,

    PlayerIdle,
};

enum class SpriteArms : uint8_t
{
    None = 0,

    PlayerIdle,
    PlayerJump,
    PlayerAttack,
};

enum class SpriteLegs : uint8_t
{
    None = 0,

    PlayerIdle,
    PlayerWalk,
    PlayerJump,
};

enum class SpriteExtra : uint8_t
{
    None = 0,

    PlayerScarfWalk,
    PlayerScarfFall,
};

enum class SpriteTile : uint8_t
{
    None = 0,

    Brick,
};

using Sprites = seb_engine::Sprites<MAX_ENTITIES, SpriteBase, SpriteHead, SpriteArms, SpriteLegs, SpriteExtra>;

template <typename S>
concept IsEntitySpriteEnum
    = std::is_same_v<S, SpriteBase> || std::is_same_v<S, SpriteHead> || std::is_same_v<S, SpriteArms>
      || std::is_same_v<S, SpriteLegs> || std::is_same_v<S, SpriteExtra>;

namespace sprites
{
auto lookup_set_movement_sprites(Sprites& sprites, size_t id, Entity entity, raylib::Vector2 vel) -> void;
[[nodiscard]] auto alternate_frame_y_offset(SpriteLegs legs) -> float;
[[nodiscard]] auto flipped_x_offset(raylib::Vector2 sprite_size) -> float;
} // namespace sprites

#endif
