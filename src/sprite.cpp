#include "sprite.hpp"

#include "components.hpp"
#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep

namespace rl = raylib;
namespace se = seb_engine;

namespace sprites
{
se::SpriteDetails sprite_details(const SpriteBase sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteBase::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteBase::PlayerIdle:
        return {
            .pos = { 128.0, 32.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteBase::Projectile:
        return {
            .pos = { 128.0, 224.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteBase::EnemyDuck:
        return {
            .pos = { 128.0, 256.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };

    // tiles
    case SpriteBase::TileBrick:
        return {
            .pos = { 0.0, 0.0 },
            .size = { TILE_SIZE, TILE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
};

se::SpriteDetails sprite_details(const SpriteHead sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteHead::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteHead::PlayerIdle:
        return {
            .pos = { 128.0, 0.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

se::SpriteDetails sprite_details(const SpriteArms sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteArms::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteArms::PlayerIdle:
        return {
            .pos = { 128.0, 64.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteArms::PlayerJump:
        return {
            .pos = { 160.0, 64.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteArms::PlayerAttack:
        return {
            .pos = { 192.0, 64.0 },
            .size = { 2 * SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.3,
            .allow_movement_override = false,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

se::SpriteDetails sprite_details(const SpriteLegs sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteLegs::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteLegs::PlayerIdle:
        return {
            .pos = { 128.0, 96.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 2,
            .frame_duration = 0.5,
            .allow_movement_override = true,
        };
    case SpriteLegs::PlayerWalk:
        return {
            .pos = { 128.0, 128.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 4,
            .frame_duration = 0.16,
            .allow_movement_override = true,
        };
    case SpriteLegs::PlayerJump:
        return {
            .pos = { 128.0, 96.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

se::SpriteDetails sprite_details(const SpriteExtra sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteExtra::None:
        return {
            .pos = { 0.0, 0.0 },
            .size = { 0.0, 0.0 },
            .frames = 1,
            .frame_duration = 0.0,
            .allow_movement_override = true,
        };
    case SpriteExtra::PlayerScarfWalk:
        return {
            .pos = { 128.0, 160.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 4,
            .frame_duration = 0.16,
            .allow_movement_override = true,
        };
    case SpriteExtra::PlayerScarfFall:
        return {
            .pos = { 128.0, 192.0 },
            .size = { SPRITE_SIZE, SPRITE_SIZE },
            .frames = 4,
            .frame_duration = 0.1,
            .allow_movement_override = true,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace sprites

void Sprites::check_update_frames(const float dt)
{
    base.check_update_frame(dt);
    head.check_update_frame(dt);
    arms.check_update_frame(dt);
    legs.check_update_frame(dt);
    extra.check_update_frame(dt);
}

void Sprites::draw(rl::Texture const& texture_sheet, const Tform transform, const bool flipped)
{
    const float y_offset = (legs.current_frame() % 2 != 0 ? alternate_frame_y_offset() : 0.0F);
    const auto pos = transform.pos + rl::Vector2(0.0, y_offset);
    const auto legs_pos = transform.pos;
    texture_sheet.Draw(base.sprite(flipped), render_pos(base, pos, flipped));
    texture_sheet.Draw(head.sprite(flipped), render_pos(head, pos, flipped));
    texture_sheet.Draw(arms.sprite(flipped), render_pos(arms, pos, flipped));
    texture_sheet.Draw(legs.sprite(flipped), render_pos(legs, legs_pos, flipped));
    texture_sheet.Draw(extra.sprite(flipped), render_pos(extra, pos, flipped));
}

void Sprites::lookup_set_movement_sprites(const Entity entity, const rl::Vector2 vel)
{
    vel != rl::Vector2(0.0, 0.0) ? lookup_set_walk_sprites(entity) : lookup_set_idle_sprites(entity);
}

float Sprites::alternate_frame_y_offset() const
{
    switch (legs.sprite())
    {
    case SpriteLegs::None:
    case SpriteLegs::PlayerJump:
        return 0.0;
    case SpriteLegs::PlayerIdle:
    case SpriteLegs::PlayerWalk:
        return 1.0;
    }

    std::unreachable();
}

void Sprites::lookup_set_fall_sprites(const Entity entity)
{
    switch (entity)
    {
    case Entity::Player:
        base.movement_set(SpriteBase::PlayerIdle);
        head.movement_set(SpriteHead::PlayerIdle);
        arms.movement_set(SpriteArms::PlayerJump);
        legs.movement_set(SpriteLegs::PlayerJump);
        extra.movement_set(SpriteExtra::PlayerScarfFall);
        break;
    default:
        break;
    }
}

void Sprites::lookup_set_jump_sprites(const Entity entity)
{
    switch (entity)
    {
    case Entity::Player:
        base.movement_set(SpriteBase::PlayerIdle);
        head.movement_set(SpriteHead::PlayerIdle);
        arms.movement_set(SpriteArms::PlayerJump);
        legs.movement_set(SpriteLegs::PlayerJump);
        extra.movement_set(SpriteExtra::None);
        break;
    default:
        break;
    }
}

void Sprites::lookup_set_walk_sprites(const Entity entity)
{
    switch (entity)
    {
    case Entity::Player:
        base.movement_set(SpriteBase::PlayerIdle);
        head.movement_set(SpriteHead::PlayerIdle);
        arms.movement_set(SpriteArms::PlayerIdle);
        legs.movement_set(SpriteLegs::PlayerWalk);
        extra.movement_set(SpriteExtra::PlayerScarfWalk);
        break;
    default:
        break;
    }
}

void Sprites::lookup_set_idle_sprites(const Entity entity)
{
    switch (entity)
    {
    case Entity::Player:
        base.movement_set(SpriteBase::PlayerIdle);
        head.movement_set(SpriteHead::PlayerIdle);
        arms.movement_set(SpriteArms::PlayerIdle);
        legs.movement_set(SpriteLegs::PlayerIdle);
        extra.movement_set(SpriteExtra::None);
        break;
    default:
        break;
    }
}
