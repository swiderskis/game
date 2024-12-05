#include "components.hpp"

#include "entities.hpp"
#include "settings.hpp"
#include "utils.hpp"

#include <cassert>
#include <optional>
#include <utility>

inline constexpr auto PLAYER_CBOX_SIZE = SimpleVec2(20.0, 29.0);
inline constexpr auto ENEMY_CBOX_SIZE = SimpleVec2(30.0, 24.0);
inline constexpr auto PLAYER_HITBOX_SIZE = SimpleVec2(12.0, 21.0);
inline constexpr auto PLAYER_HITBOX_OFFSET = SimpleVec2(0.0, 4.0);
inline constexpr auto ENEMY_HITBOX_SIZE = SimpleVec2(22.0, 16.0);
inline constexpr auto ENEMY_HITBOX_OFFSET = SimpleVec2(0.0, 4.0);
inline constexpr auto MELEE_HITBOX_SIZE = SimpleVec2(18.0, 7.0);
inline constexpr auto MELEE_HITBOX_OFFSET = SimpleVec2(24.0, 9);

inline constexpr float PROJECTILE_SPEED = 500.0;
inline constexpr float PROJECTILE_LIFESPAN = 0.3;

inline constexpr int RECTANGLE_BBOX_INDEX = 0;
inline constexpr int PLAYER_HEALTH = 100;
inline constexpr int ENEMY_HEALTH = 100;

void Sprite::check_update_frames(const float dt)
{
    base.check_update_frame(dt);
    head.check_update_frame(dt);
    arms.check_update_frame(dt);
    legs.check_update_frame(dt);
    extra.check_update_frame(dt);
}

void Sprite::draw(RTexture const& texture_sheet, const Tform transform, const bool flipped)
{
    const float y_offset = (legs.current_frame() % 2 != 0 ? alternate_frame_y_offset() : 0.0F);
    const auto pos = transform.pos + RVector2(0.0, y_offset);
    const auto legs_pos = transform.pos;
    texture_sheet.Draw(base.sprite(flipped), render_pos(base, pos, flipped));
    texture_sheet.Draw(head.sprite(flipped), render_pos(head, pos, flipped));
    texture_sheet.Draw(arms.sprite(flipped), render_pos(arms, pos, flipped));
    texture_sheet.Draw(legs.sprite(flipped), render_pos(legs, legs_pos, flipped));
    texture_sheet.Draw(extra.sprite(flipped), render_pos(extra, pos, flipped));
}

void Sprite::lookup_set_movement_parts(const Entity entity, const RVector2 vel)
{
    if (vel.y > 0)
    {
        lookup_set_fall_parts(entity);
    }
    else if (vel.y < 0)
    {
        lookup_set_jump_parts(entity);
    }
    else if (vel.x != 0)
    {
        lookup_set_walk_parts(entity);
    }
    else
    {
        lookup_set_idle_parts(entity);
    }
}

float Sprite::alternate_frame_y_offset() const
{
    switch (legs.part())
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

void Sprite::lookup_set_fall_parts(const Entity entity)
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
    case Entity::Tile:
    case Entity::Projectile:
    case Entity::Enemy:
    case Entity::Melee:
        break;
    }
}

void Sprite::lookup_set_jump_parts(const Entity entity)
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
    case Entity::Tile:
    case Entity::Projectile:
    case Entity::Enemy:
    case Entity::Melee:
        break;
    }
}

void Sprite::lookup_set_walk_parts(const Entity entity)
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
    case Entity::Tile:
    case Entity::Projectile:
    case Entity::Enemy:
    case Entity::Melee:
        break;
    }
}

void Sprite::lookup_set_idle_parts(const Entity entity)
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
    case Entity::Tile:
    case Entity::Projectile:
    case Entity::Enemy:
    case Entity::Melee:
        break;
    }
}

Circle::Circle(const RVector2 pos, const float radius) : pos(pos), radius(radius)
{
}

bool Circle::check_collision(const Circle other_circle) const
{
    return radius + other_circle.radius > pos.Distance(other_circle.pos);
}

void Circle::draw_lines(const ::Color color) const
{
    DrawCircleLines((int)pos.x, (int)pos.y, radius, color);
}

void BBox::sync(const Tform transform, const bool flipped)
{
    std::visit(
        overloaded{
            [transform, this, flipped](RRectangle& bbox)
            {
                bbox.SetPosition(transform.pos);
                bbox.x += (SPRITE_SIZE - bbox.width) / 2;
                bbox.y += (SPRITE_SIZE - bbox.height);
                bbox.x += offset.x * (flipped ? -1.0F : 1.0F);
                bbox.y -= offset.y;
            },
            [transform, this, flipped](Circle& bbox)
            {
                bbox.pos = transform.pos;
                bbox.pos.x += SPRITE_SIZE / 2;
                bbox.pos.y += SPRITE_SIZE / 2;
                bbox.pos.x += offset.x * (flipped ? -1.0F : 1.0F);
                bbox.pos.y -= offset.y;
            },
        },
        m_bounding_box);
}

bool BBox::collides(const BBox other_bbox) const
{
    const auto rect_bbox = [other_bbox](RRectangle bbox)
    {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return bbox.CheckCollision(other_bbox); },
                [bbox](const Circle other_bbox) { return bbox.CheckCollision(other_bbox.pos, other_bbox.radius); },
            },
            other_bbox.m_bounding_box);
    };
    const auto circle_bbox = [other_bbox](Circle bbox)

    {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return other_bbox.CheckCollision(bbox.pos, bbox.radius); },
                [bbox](const Circle other_bbox) { return bbox.check_collision(other_bbox); },
            },
            other_bbox.m_bounding_box);
    };

    return std::visit(
        overloaded{
            rect_bbox,
            circle_bbox,
        },
        m_bounding_box);
}

bool BBox::x_overlaps(const BBox other_bbox) const
{
    assert(m_bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return std::visit(
        overloaded{
            [bbox](const RRectangle other_bbox)
            {
                return (bbox.x >= other_bbox.x && bbox.x - other_bbox.x < other_bbox.width)
                       || (other_bbox.x >= bbox.x && other_bbox.x - bbox.x < bbox.width);
            },
            [bbox](const Circle other_bbox)
            {
                return (bbox.x >= other_bbox.pos.x && other_bbox.pos.x + other_bbox.radius > bbox.x)
                       || (other_bbox.pos.x >= bbox.x && bbox.x + bbox.width > other_bbox.pos.x - other_bbox.radius);
            },
        },
        other_bbox.m_bounding_box);
}

bool BBox::y_overlaps(const BBox other_bbox) const
{
    assert(m_bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return std::visit(
        overloaded{
            [bbox](const RRectangle other_bbox)
            {
                return (bbox.y >= other_bbox.y && bbox.y - other_bbox.y < other_bbox.height)
                       || (other_bbox.y >= bbox.y && other_bbox.y - bbox.y < bbox.height);
            },
            [bbox](const Circle other_bbox)
            {
                return (bbox.y >= other_bbox.pos.y && other_bbox.pos.y + other_bbox.radius > bbox.y)
                       || (other_bbox.pos.y >= bbox.y && bbox.y + bbox.height > other_bbox.pos.y - other_bbox.radius);
            },
        },
        other_bbox.m_bounding_box);
}

void BBox::set(const Tform transform, const RVector2 size)
{
    m_bounding_box = RRectangle(RVector2(0.0, 0.0), size);
    sync(transform, false);
}

void BBox::set(const Tform transform, const float radius)
{
    m_bounding_box = Circle(RVector2(0.0, 0.0), radius);
    sync(transform, false);
}

BBoxVariant BBox::bounding_box() const
{
    return m_bounding_box;
}

void Health::set_health(const int health)
{
    current = health;
    max = health;
}

float Health::percentage() const
{
    assert(max != std::nullopt);

    return (float)current / (float)max.value();
}

Components::Components()
{
    flags.resize(MAX_ENTITIES, 0);
    lifespans.resize(MAX_ENTITIES, std::nullopt);
    parents.resize(MAX_ENTITIES, std::nullopt);
}

void Components::init_player(const unsigned id, const RVector2 pos)
{
    transforms[id].pos = pos;
    collision_boxes[id].set(transforms[id], PLAYER_CBOX_SIZE);
    healths[id].set_health(PLAYER_HEALTH);
    hitboxes[id].set(transforms[id], PLAYER_HITBOX_SIZE);
    hitboxes[id].offset = PLAYER_HITBOX_OFFSET;
}

void Components::init_tile(const unsigned id, const RVector2 pos, const Tile tile)
{
    transforms[id].pos = pos;
    collision_boxes[id].set(transforms[id], RVector2(TILE_SIZE, TILE_SIZE));

    switch (tile)
    {
    case Tile::Brick:
        sprites[id].base.set(SpriteBase::TileBrick);
        break;
    }
}

void Components::init_projectile(const unsigned id, const RVector2 source_pos, const RVector2 target_pos)
{
    const auto diff = target_pos - source_pos;
    const float angle = atan2(diff.y, diff.x);
    transforms[id].pos = source_pos;
    transforms[id].vel = RVector2(cos(angle), sin(angle)) * PROJECTILE_SPEED;
    sprites[id].base.set(SpriteBase::Projectile);
    collision_boxes[id].set(transforms[id], 4);
    lifespans[id] = PROJECTILE_LIFESPAN;
    hitboxes[id].set(transforms[id], 4);
}

void Components::init_enemy(const unsigned id, const RVector2 pos, const Enemy enemy)
{
    transforms[id].pos = pos;
    collision_boxes[id].set(transforms[id], ENEMY_CBOX_SIZE);
    healths[id].set_health(ENEMY_HEALTH);
    hitboxes[id].set(transforms[id], ENEMY_HITBOX_SIZE);
    hitboxes[id].offset = ENEMY_HITBOX_OFFSET;

    switch (enemy)
    {
    case Enemy::Duck:
        sprites[id].base.set(SpriteBase::EnemyDuck);
        break;
    }
}

void Components::init_melee(const unsigned id, const RVector2 pos, const unsigned parent_id)
{
    transforms[id].pos = pos;
    collision_boxes[id].set(transforms[id], RVector2(0.0, 0.0));
    lifespans[id] = components::sprite_details(SpriteArms::PlayerAttack).frame_duration;
    hitboxes[id].set(transforms[id], MELEE_HITBOX_SIZE);
    hitboxes[id].offset = MELEE_HITBOX_OFFSET;
    parents[id] = parent_id;
}

void Components::uninit_destroyed_entity(const unsigned id)
{
    transforms[id].vel = RVector2(0.0, 0.0);
    sprites[id].base.set(SpriteBase::None);
    sprites[id].head.set(SpriteHead::None);
    sprites[id].arms.set(SpriteArms::None);
    sprites[id].legs.set(SpriteLegs::None);
    sprites[id].extra.set(SpriteExtra::None);
    collision_boxes[id].offset = RVector2(0.0, 0.0);
    flags[id] = 0;
    lifespans[id] = std::nullopt;
    healths[id].max = std::nullopt;
    hitboxes[id].offset = RVector2(0.0, 0.0);
    parents[id] = std::nullopt;
}

namespace components
{
SpriteDetails sprite_details(const SpriteBase sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteBase::None:
        return { { 0.0, 0.0 }, { 0.0, 0.0 }, 1, 0.0, true };
    case SpriteBase::PlayerIdle:
        return { { 128.0, 32.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    case SpriteBase::Projectile:
        return { { 128.0, 224.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    case SpriteBase::EnemyDuck:
        return { { 128.0, 256.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };

    // tiles
    case SpriteBase::TileBrick:
        return { { 0.0, 0.0 }, { TILE_SIZE, TILE_SIZE }, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteHead sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteHead::None:
        return { { 0.0, 0.0 }, { 0.0, 0.0 }, 1, 0.0, true };
    case SpriteHead::PlayerIdle:
        return { { 128.0, 0.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteArms sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteArms::None:
        return { { 0.0, 0.0 }, { 0.0, 0.0 }, 1, 0.0, true };
    case SpriteArms::PlayerIdle:
        return { { 128.0, 64.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    case SpriteArms::PlayerJump:
        return { { 160.0, 64.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    case SpriteArms::PlayerAttack:
        return { { 192.0, 64.0 }, { 2 * SPRITE_SIZE, SPRITE_SIZE }, 1, 0.3, false };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteLegs sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteLegs::None:
        return { { 0.0, 0.0 }, { 0.0, 0.0 }, 1, 0.0, true };
    case SpriteLegs::PlayerIdle:
        return { { 128.0, 96.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 2, 0.5, true };
    case SpriteLegs::PlayerWalk:
        return { { 128.0, 128.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 4, 0.16, true };
    case SpriteLegs::PlayerJump:
        return { { 128.0, 96.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteExtra sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteExtra::None:
        return { { 0.0, 0.0 }, { 0.0, 0.0 }, 1, 0.0, true };
    case SpriteExtra::PlayerScarfWalk:
        return { { 128.0, 160.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 4, 0.16, true };
    case SpriteExtra::PlayerScarfFall:
        return { { 128.0, 192.0 }, { SPRITE_SIZE, SPRITE_SIZE }, 4, 0.1, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace components
