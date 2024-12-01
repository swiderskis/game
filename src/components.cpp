#include "components.hpp"

#include "overloaded.hpp"

#include <cassert>
#include <optional>
#include <utility>

inline constexpr int RECTANGLE_BBOX_INDEX = 0;

void Sprite::check_update_frames(const float dt)
{
    base.check_update_frame(dt);
    head.check_update_frame(dt);
    arms.check_update_frame(dt);
    legs.check_update_frame(dt);
    extra.check_update_frame(dt);
}

void Sprite::draw(RTexture const& texture_sheet, const Tform transform)
{
    if (transform.vel.x != 0)
    {
        flipped = transform.vel.x < 0;
    }

    const float y_offset = (legs.current_frame() % 2 != 0 ? alternate_frame_y_offset() : 0.0F);
    const auto pos = transform.pos + RVector2(0.0, y_offset);
    const auto leg_pos = transform.pos;
    texture_sheet.Draw(base.sprite(flipped), pos);
    texture_sheet.Draw(head.sprite(flipped), pos);
    texture_sheet.Draw(arms.sprite(flipped), pos);
    texture_sheet.Draw(legs.sprite(flipped), leg_pos);
    texture_sheet.Draw(extra.sprite(flipped), pos);
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

void BBox::sync(const Tform transform)
{
    std::visit(
        overloaded{
            [transform](RRectangle& bbox)
            {
                bbox.SetPosition(transform.pos);
                bbox.x += (TILE_SIZE - bbox.width) / 2;
                bbox.y += (TILE_SIZE - bbox.height);
            },
            [transform](Circle& bbox)
            {
                bbox.pos = transform.pos;
                bbox.pos.x += TILE_SIZE / 2;
                bbox.pos.y += TILE_SIZE / 2;
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
    sync(transform);
}

void BBox::set(const Tform transform, const float radius)
{
    m_bounding_box = Circle(RVector2(0.0, 0.0), radius);
    sync(transform);
}

std::variant<RRectangle, Circle> BBox::bounding_box() const
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

namespace components
{
SpriteDetails sprite_details(const SpriteBase sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteBase::None:
        return { 0.0, 0.0, 0.0, 1, 0.0, true };
    case SpriteBase::PlayerIdle:
        return { 128.0, 32.0, TILE_SIZE, 1, 0.0, true };
    case SpriteBase::Projectile:
        return { 128.0, 224.0, TILE_SIZE, 1, 0.0, true };
    case SpriteBase::Enemy:
        return { 128.0, 256.0, TILE_SIZE, 1, 0.0, true };

    // tiles
    case SpriteBase::TileBrick:
        return { 0.0, 0.0, TILE_SIZE, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteHead sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteHead::None:
        return { 0.0, 0.0, 0.0, 1, 0.0, true };
    case SpriteHead::PlayerIdle:
        return { 128.0, 0.0, TILE_SIZE, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteArms sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteArms::None:
        return { 0.0, 0.0, 0.0, 1, 0.0, true };
    case SpriteArms::PlayerIdle:
        return { 128.0, 64.0, TILE_SIZE, 1, 0.0, true };
    case SpriteArms::PlayerJump:
        return { 160.0, 64.0, TILE_SIZE, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteLegs sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteLegs::None:
        return { 0.0, 0.0, 0.0, 1, 0.0, true };
    case SpriteLegs::PlayerIdle:
        return { 128.0, 96.0, TILE_SIZE, 2, 0.5, true };
    case SpriteLegs::PlayerWalk:
        return { 128.0, 128.0, TILE_SIZE, 4, 0.16, true };
    case SpriteLegs::PlayerJump:
        return { 128.0, 96.0, TILE_SIZE, 1, 0.0, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}

SpriteDetails sprite_details(const SpriteExtra sprite)
{
    switch (sprite)
    { // NOLINTBEGIN(*magic-numbers)
    case SpriteExtra::None:
        return { 0.0, 0.0, 0.0, 1, 0.0, true };
    case SpriteExtra::PlayerScarfWalk:
        return { 128.0, 160.0, TILE_SIZE, 4, 0.16, true };
    case SpriteExtra::PlayerScarfFall:
        return { 128.0, 192.0, TILE_SIZE, 4, 0.1, true };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace components
