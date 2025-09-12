#include "sprites.hpp"

#include "entities.hpp"
#include "se-sprite.hpp"
#include "tiles.hpp"

#include <utility>

namespace rl = raylib;
namespace se = seb_engine;

namespace
{
auto lookup_set_walk_sprites(Sprites& sprites, size_t id, Entity entity) -> void;
auto lookup_set_idle_sprites(Sprites& sprites, size_t id, Entity entity) -> void;
} // namespace

template <>
auto se::SpriteDetailsLookup<SpriteBase>::get(SpriteBase sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteBase::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteBase::PlayerIdle:
        return {
            .pos = { 128.0, 32.0 },
            .size = SPRITE_SIZE,
        };
    case SpriteBase::Projectile:
        return {
            .pos = { 128.0, 224.0 },
            .size = SPRITE_SIZE,
        };
    case SpriteBase::EnemyDuck:
        return {
            .pos = { 128.0, 256.0 },
            .size = SPRITE_SIZE,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

template <>
auto se::SpriteDetailsLookup<SpriteHead>::get(SpriteHead sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteHead::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteHead::PlayerIdle:
        return {
            .pos = { 128.0, 0.0 },
            .size = SPRITE_SIZE,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

template <>
auto se::SpriteDetailsLookup<SpriteArms>::get(SpriteArms sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteArms::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteArms::PlayerIdle:
        return {
            .pos = { 128.0, 64.0 },
            .size = SPRITE_SIZE,
        };
    case SpriteArms::PlayerJump:
        return {
            .pos = { 160.0, 64.0 },
            .size = SPRITE_SIZE,
        };
    case SpriteArms::PlayerAttack:
        return {
            .pos = { 192.0, 64.0 },
            .size = { 2 * SPRITE_LEN, SPRITE_LEN },
            .frame_duration = 0.3,
            .allow_movement_override = false,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

template <>
auto se::SpriteDetailsLookup<SpriteLegs>::get(SpriteLegs sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteLegs::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteLegs::PlayerIdle:
        return {
            .pos = { 128.0, 96.0 },
            .size = SPRITE_SIZE,
            .frames = 2,
            .frame_duration = 0.5,
        };
    case SpriteLegs::PlayerWalk:
        return {
            .pos = { 128.0, 128.0 },
            .size = SPRITE_SIZE,
            .frames = 4,
            .frame_duration = 0.16,
        };
    case SpriteLegs::PlayerJump:
        return {
            .pos = { 128.0, 96.0 },
            .size = SPRITE_SIZE,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

template <>
auto se::SpriteDetailsLookup<SpriteExtra>::get(SpriteExtra sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteExtra::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteExtra::PlayerScarfWalk:
        return {
            .pos = { 128.0, 160.0 },
            .size = SPRITE_SIZE,
            .frames = 4,
            .frame_duration = 0.16,
        };
    case SpriteExtra::PlayerScarfFall:
        return {
            .pos = { 128.0, 192.0 },
            .size = { SPRITE_SIZE },
            .frames = 4,
            .frame_duration = 0.1,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

template <>
auto se::SpriteDetailsLookup<SpriteTile>::get(SpriteTile sprite) -> se::SpriteDetails
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteTile::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
        };
    case SpriteTile::Brick:
        return {
            .pos = { 0.0, 0.0 },
            .size = TILE_SIZE,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

namespace sprites
{
auto lookup_set_movement_sprites(Sprites& sprites, const size_t id, const Entity entity, const rl::Vector2 vel) -> void
{
    vel != rl::Vector2(0.0, 0.0) ? lookup_set_walk_sprites(sprites, id, entity)
                                 : lookup_set_idle_sprites(sprites, id, entity);
}

auto alternate_frame_y_offset(const SpriteLegs legs) -> float
{
    switch (legs)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteLegs::None:
    case SpriteLegs::PlayerJump:
        return 0.0;
    case SpriteLegs::PlayerIdle:
    case SpriteLegs::PlayerWalk:
        return 1.0;
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

auto flipped_x_offset(const rl::Vector2 sprite_size) -> float
{
    return SPRITE_LEN - sprite_size.x;
}
} // namespace sprites

namespace
{
auto lookup_set_walk_sprites(Sprites& sprites, const size_t id, const Entity entity) -> void
{
    switch (entity)
    {
    case Entity::Player:
        sprites.by_id(id)
            .movement_set(SpriteBase::PlayerIdle)
            .movement_set(SpriteHead::PlayerIdle)
            .movement_set(SpriteArms::PlayerIdle)
            .movement_set(SpriteLegs::PlayerWalk)
            .movement_set(SpriteExtra::PlayerScarfWalk);
        break;
    default:
        break;
    }
}

auto lookup_set_idle_sprites(Sprites& sprites, const size_t id, const Entity entity) -> void
{
    switch (entity)
    {
    case Entity::Player:
        sprites.by_id(id)
            .movement_set(SpriteBase::PlayerIdle)
            .movement_set(SpriteHead::PlayerIdle)
            .movement_set(SpriteArms::PlayerIdle)
            .movement_set(SpriteLegs::PlayerIdle)
            .movement_set(SpriteExtra::None);
        break;
    default:
        break;
    }
}
} // namespace
