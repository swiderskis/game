#include "components.hpp"

#include "entities.hpp"
#include "seblib.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <utility>

namespace sl = seblib;
namespace slog = seblib::log;
namespace sm = seblib::math;

namespace
{
bool check_collision(RRectangle rectangle1, RRectangle rectangle2);
bool check_collision(RRectangle rectangle, sm::Circle circle);
bool check_collision(RRectangle rectangle, sm::Line line);
bool check_collision(sm::Circle circle, RRectangle rectangle);
bool check_collision(sm::Circle circle1, sm::Circle circle2);
bool check_collision(sm::Circle circle, sm::Line line);
bool check_collision(sm::Line line, RRectangle rectangle);
bool check_collision(sm::Line line, sm::Circle circle);
bool check_collision(sm::Line line1, sm::Line line2);
} // namespace

void BBox::sync(const RVector2 pos, const bool flipped)
{
    sl::match(
        m_bbox,
        [pos, this, flipped](RRectangle& bbox)
        {
            bbox.SetPosition(pos);
            bbox.x += (SPRITE_SIZE - bbox.width) / 2;
            bbox.y += (SPRITE_SIZE - bbox.height);
            bbox.x += m_offset.x * (flipped ? -1.0F : 1.0F);
            bbox.y -= m_offset.y;
            slog::log(slog::TRC, "Rectangle bbox pos: ({}, {})", bbox.x, bbox.y);
        },
        [pos, this, flipped](sm::Circle& bbox)
        {
            bbox.pos = pos;
            bbox.pos.x += SPRITE_SIZE / 2;
            bbox.pos.y += SPRITE_SIZE / 2;
            bbox.pos.x += m_offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos.y -= m_offset.y;
            slog::log(slog::TRC, "Circle bbox pos: ({}, {})", bbox.pos.x, bbox.pos.y);
        },
        [pos, this, flipped](sm::Line& bbox)
        {
            bbox.pos2 = pos + (bbox.pos2 - bbox.pos1);
            bbox.pos1 = pos;
            bbox.pos1.x += m_offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos1.y += m_offset.y * (flipped ? -1.0F : 1.0F);
            bbox.pos2.x += m_offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos2.y += m_offset.y * (flipped ? -1.0F : 1.0F);
            bbox.pos1.x += SPRITE_SIZE / 2;
            bbox.pos1.y += SPRITE_SIZE / 2;
            bbox.pos2.x += SPRITE_SIZE / 2;
            bbox.pos2.y += SPRITE_SIZE / 2;
            slog::log(slog::TRC, "Line bbox pos 1: ({}, {})", bbox.pos1.x, bbox.pos1.y);
            slog::log(slog::TRC, "Line bbox pos 2: ({}, {})", bbox.pos2.x, bbox.pos2.y);
        });
}

bool BBox::collides(const BBox other_bbox) const
{
    return sl::match(m_bbox,
                     [other_bbox](const auto bbox)
                     {
                         return sl::match(other_bbox.m_bbox,
                                          [bbox](const auto other_bbox) { return check_collision(bbox, other_bbox); });
                     });
}

bool BBox::x_overlaps(const BBox other_bbox) const
{
    // method currently should only be used for tile collision correction, tiles always have rectangular bboxes
    assert(m_bbox.index() == RECTANGLE);

    const auto bbox = std::get<RRectangle>(m_bbox);

    return sl::match(
        other_bbox.m_bbox,
        [bbox](const RRectangle other_bbox)
        {
            return (bbox.x >= other_bbox.x && bbox.x - other_bbox.x < other_bbox.width)
                   || (other_bbox.x >= bbox.x && other_bbox.x - bbox.x < bbox.width);
        },
        [bbox](const sm::Circle other_bbox)
        {
            return (bbox.x >= other_bbox.pos.x && other_bbox.pos.x + other_bbox.radius > bbox.x)
                   || (other_bbox.pos.x >= bbox.x && bbox.x + bbox.width > other_bbox.pos.x - other_bbox.radius);
        },
        [bbox](const sm::Line other_bbox)
        {
            return (bbox.x <= other_bbox.pos1.x && bbox.x + bbox.width >= other_bbox.pos1.x)
                   || (bbox.x <= other_bbox.pos2.x && bbox.x + bbox.width >= other_bbox.pos2.x)
                   || (bbox.x >= other_bbox.pos1.x && bbox.x + bbox.width <= other_bbox.pos2.x)
                   || (bbox.x >= other_bbox.pos2.x && bbox.x + bbox.width <= other_bbox.pos1.x);
        });
}

bool BBox::y_overlaps(const BBox other_bbox) const
{
    // method currently should only be used for tile collision correction, tiles always have rectangular bboxes
    assert(m_bbox.index() == RECTANGLE);

    const auto bbox = std::get<RRectangle>(m_bbox);

    return sl::match(
        other_bbox.m_bbox,
        [bbox](const RRectangle other_bbox)
        {
            return (bbox.y >= other_bbox.y && bbox.y - other_bbox.y < other_bbox.height)
                   || (other_bbox.y >= bbox.y && other_bbox.y - bbox.y < bbox.height);
        },
        [bbox](const sm::Circle other_bbox)
        {
            return (bbox.y >= other_bbox.pos.y && other_bbox.pos.y + other_bbox.radius > bbox.y)
                   || (other_bbox.pos.y >= bbox.y && bbox.y + bbox.height > other_bbox.pos.y - other_bbox.radius);
        },
        [bbox](const sm::Line other_bbox)
        {
            return (bbox.y <= other_bbox.pos1.y && bbox.y + bbox.height >= other_bbox.pos1.y)
                   || (bbox.y <= other_bbox.pos2.y && bbox.y + bbox.height >= other_bbox.pos2.y)
                   || (bbox.y >= other_bbox.pos1.y && bbox.y + bbox.height <= other_bbox.pos2.y)
                   || (bbox.y >= other_bbox.pos2.y && bbox.y + bbox.height <= other_bbox.pos1.y);
        });
}

void BBox::set(const RVector2 pos, const RVector2 size)
{
    m_bbox = RRectangle(RVector2(0.0, 0.0), size);
    sync(pos, false);
}

void BBox::set(const RVector2 pos, const float radius)
{
    m_bbox = sm::Circle(RVector2(0.0, 0.0), radius);
    sync(pos, false);
}

void BBox::set(const RVector2 pos, const float len, const float angle)
{
    m_bbox = sm::Line(RVector2(0.0, 0.0), len, angle);
    sync(pos, false);
}

void BBox::set_offset(RVector2 pos, RVector2 offset)
{
    m_offset = offset;
    sync(pos, false);
}

BBoxVariant BBox::bbox() const
{
    return m_bbox;
}

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
    vel != RVector2(0.0, 0.0) ? lookup_set_walk_parts(entity) : lookup_set_idle_parts(entity);
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
    default:
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
    default:
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
    default:
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
    default:
        break;
    }
}

bool Flags::is_enabled(const Flag flag_enum) const
{
    return flag[flag_enum];
}

void Flags::set(const Flag flag_enum, const bool val)
{
    flag[flag_enum] = val;
}

void Health::set(const int health)
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
}

SpriteDetails sprite_details(const SpriteHead sprite)
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

SpriteDetails sprite_details(const SpriteArms sprite)
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

SpriteDetails sprite_details(const SpriteLegs sprite)
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

SpriteDetails sprite_details(const SpriteExtra sprite)
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
} // namespace components

namespace
{
bool check_collision(const RRectangle rectangle1, const RRectangle rectangle2)
{
    return rectangle1.CheckCollision(rectangle2);
}

bool check_collision(const RRectangle rectangle, const sm::Circle circle)
{
    return rectangle.CheckCollision(circle.pos, circle.radius);
}

bool check_collision(const RRectangle rectangle, const sm::Line line)
{
    const auto rect_pos = rectangle.GetPosition();
    const auto rect_size = rectangle.GetSize();
    const auto rect_line1 = sm::Line(rect_pos, rect_pos + RVector2(rectangle.width, 0.0));
    const auto rect_line2 = sm::Line(rect_pos, rect_pos + RVector2(0.0, rectangle.height));
    const auto rect_line3 = sm::Line(rect_pos + RVector2(rectangle.width, 0.0), rect_pos + rect_size);
    const auto rect_line4 = sm::Line(rect_pos + RVector2(0.0, rectangle.height), rect_pos + rect_size);

    return rectangle.CheckCollision(line.pos1) || rectangle.CheckCollision(line.pos2)
           || check_collision(rect_line1, line) || check_collision(rect_line2, line)
           || check_collision(rect_line3, line) || check_collision(rect_line4, line);
}

bool check_collision(const sm::Circle circle, const RRectangle rectangle)
{
    return check_collision(rectangle, circle);
}

bool check_collision(const sm::Circle circle1, const sm::Circle circle2)
{
    return circle1.radius + circle2.radius > circle1.pos.Distance(circle2.pos);
}

bool check_collision(const sm::Circle circle, const sm::Line line)
{
    return CheckCollisionCircleLine(circle.pos, circle.radius, line.pos1, line.pos2);
}

bool check_collision(const sm::Line line, const RRectangle rectangle)
{
    return check_collision(rectangle, line);
}

bool check_collision(const sm::Line line, const sm::Circle circle)
{
    return check_collision(circle, line);
}

bool check_collision(const sm::Line line1, const sm::Line line2)
{
    return line1.pos1.CheckCollisionLines(line1.pos2, line2.pos1, line2.pos2, nullptr);
}
} // namespace
