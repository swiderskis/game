#include "components.hpp"

#include "entities.hpp"
#include "logging.hpp"
#include "seblib.hpp"
#include "settings.hpp"

#include <cassert>
#include <cmath>
#include <optional>
#include <utility>

namespace sl = seblib;

namespace
{
bool check_collision(RRectangle rectangle1, RRectangle rectangle2);
bool check_collision(RRectangle rectangle, Circle circle);
bool check_collision(RRectangle rectangle, Line line);
bool check_collision(Circle circle, RRectangle rectangle);
bool check_collision(Circle circle1, Circle circle2);
bool check_collision(Circle circle, Line line);
bool check_collision(Line line, RRectangle rectangle);
bool check_collision(Line line, Circle circle);
bool check_collision(Line line1, Line line2);
} // namespace

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

Circle::Circle(const RVector2 pos, const float radius) : pos(pos), radius(radius)
{
}

void Circle::draw_lines(const ::Color color) const
{
    DrawCircleLines((int)pos.x, (int)pos.y, radius, color);
}

Line::Line(const RVector2 pos1, const RVector2 pos2) : pos1(pos1), pos2(pos2)
{
}

Line::Line(const RVector2 pos, const float len, const float angle) : pos1(pos)
{
    pos2 = pos1 + RVector2(len * cos(angle), len * sin(angle));
}

float Line::len() const
{
    const float x_len = pos2.x - pos1.x;
    const float y_len = pos2.y - pos1.y;

    return sqrt((x_len * x_len) + (y_len * y_len));
}

void Line::draw_line(::Color color) const
{
    DrawLine((int)pos1.x, (int)pos1.y, (int)pos2.x, (int)pos2.y, color);
}

void BBox::sync(const Tform transform, const bool flipped)
{
    sl::match(
        m_bounding_box,
        [transform, this, flipped](RRectangle& bbox)
        {
            bbox.SetPosition(transform.pos);
            bbox.x += (SPRITE_SIZE - bbox.width) / 2;
            bbox.y += (SPRITE_SIZE - bbox.height);
            bbox.x += offset.x * (flipped ? -1.0F : 1.0F);
            bbox.y -= offset.y;
            LOG_TRC("Rectangle bbox pos: ({}, {})", bbox.x, bbox.y);
        },
        [transform, this, flipped](Circle& bbox)
        {
            bbox.pos = transform.pos;
            bbox.pos.x += SPRITE_SIZE / 2;
            bbox.pos.y += SPRITE_SIZE / 2;
            bbox.pos.x += offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos.y -= offset.y;
            LOG_TRC("Circle bbox pos: ({}, {})", bbox.pos.x, bbox.pos.y);
        },
        [transform, this, flipped](Line& bbox)
        {
            bbox.pos2 = transform.pos + (bbox.pos2 - bbox.pos1);
            bbox.pos1 = transform.pos;
            bbox.pos1.x += offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos1.y += offset.y * (flipped ? -1.0F : 1.0F);
            bbox.pos2.x += offset.x * (flipped ? -1.0F : 1.0F);
            bbox.pos2.y += offset.y * (flipped ? -1.0F : 1.0F);
            bbox.pos1.x += SPRITE_SIZE / 2;
            bbox.pos1.y += SPRITE_SIZE / 2;
            bbox.pos2.x += SPRITE_SIZE / 2;
            bbox.pos2.y += SPRITE_SIZE / 2;
            LOG_TRC("Line bbox pos 1: ({}, {})", bbox.pos1.x, bbox.pos1.y);
            LOG_TRC("Line bbox pos 2: ({}, {})", bbox.pos2.x, bbox.pos2.y);
        });
}

bool BBox::collides(const BBox other_bbox) const
{
    return sl::match(m_bounding_box,
                     [other_bbox](const auto bbox)
                     {
                         return sl::match(other_bbox.m_bounding_box,
                                          [bbox](const auto other_bbox) { return check_collision(bbox, other_bbox); });
                     });
}

bool BBox::x_overlaps(const BBox other_bbox) const
{
    // method currently should only be used for tile collision correction, tiles always have rectangular bboxes
    assert(m_bounding_box.index() == RECTANGLE);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return sl::match(
        other_bbox.m_bounding_box,
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
        [bbox](const Line other_bbox)
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
    assert(m_bounding_box.index() == RECTANGLE);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return sl::match(
        other_bbox.m_bounding_box,
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
        [bbox](const Line other_bbox)
        {
            return (bbox.y <= other_bbox.pos1.y && bbox.y + bbox.height >= other_bbox.pos1.y)
                   || (bbox.y <= other_bbox.pos2.y && bbox.y + bbox.height >= other_bbox.pos2.y)
                   || (bbox.y >= other_bbox.pos1.y && bbox.y + bbox.height <= other_bbox.pos2.y)
                   || (bbox.y >= other_bbox.pos2.y && bbox.y + bbox.height <= other_bbox.pos1.y);
        });
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

void BBox::set(const Tform transform, const float len, const float angle)
{
    m_bounding_box = Line(RVector2(0.0, 0.0), len, angle);
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
    attack_cooldowns.resize(MAX_ENTITIES, 0.0);
    invuln_times.resize(MAX_ENTITIES, 0.0);
    hit_damage.resize(MAX_ENTITIES, 0);
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        LOG_TRC("Invuln time [{}]: {}", i, invuln_times[i]);
    }
}

void Components::uninit_destroyed_entity(const unsigned id)
{
    transforms[id].vel = RVector2(0.0, 0.0);
    sprites[id].base.set(SpriteBase::None);
    sprites[id].head.set(SpriteHead::None);
    sprites[id].arms.set(SpriteArms::None);
    sprites[id].legs.set(SpriteLegs::None);
    sprites[id].extra.set(SpriteExtra::None);
    collision_boxes[id].set(transforms[id], RVector2(0.0, 0.0));
    collision_boxes[id].offset = RVector2(0.0, 0.0);
    flags[id] = 0;
    lifespans[id] = std::nullopt;
    healths[id].max = std::nullopt;
    hitboxes[id].set(transforms[id], RVector2(0.0, 0.0));
    hitboxes[id].offset = RVector2(0.0, 0.0);
    parents[id] = std::nullopt;
    invuln_times[id] = 0.0;
}

EntityComponents Components::get_by_id(unsigned id)
{
    return { *this, id };
}

EntityComponents::EntityComponents(Components& components, unsigned id) : m_components(&components), m_id(id)
{
}

EntityComponents& EntityComponents::set_pos(const RVector2 pos)
{
    m_components->transforms[m_id].pos = pos;

    return *this;
}

EntityComponents& EntityComponents::set_vel(const RVector2 vel)
{
    m_components->transforms[m_id].vel = vel;

    return *this;
}

EntityComponents& EntityComponents::set_cbox_size(const RVector2 cbox_size)
{
    const auto transform = m_components->transforms[m_id];
    m_components->collision_boxes[m_id].set(transform, cbox_size);

    return *this;
}

EntityComponents& EntityComponents::set_cbox_size(const float radius)
{
    const auto transform = m_components->transforms[m_id];
    m_components->collision_boxes[m_id].set(transform, radius);

    return *this;
}

EntityComponents& EntityComponents::set_cbox_size(const float len, const float angle)
{
    const auto transform = m_components->transforms[m_id];
    m_components->collision_boxes[m_id].set(transform, len, angle);

    return *this;
}

EntityComponents& EntityComponents::set_cbox_offset(const RVector2 offset)
{
    m_components->collision_boxes[m_id].offset = offset;

    return *this;
}

EntityComponents& EntityComponents::set_health(const int health)
{
    m_components->healths[m_id].current = health;
    m_components->healths[m_id].max = health;

    return *this;
}

EntityComponents& EntityComponents::set_hitbox_size(const RVector2 hbox_size)
{
    const auto transform = m_components->transforms[m_id];
    m_components->hitboxes[m_id].set(transform, hbox_size);

    return *this;
}

EntityComponents& EntityComponents::set_hitbox_size(const float radius)
{
    const auto transform = m_components->transforms[m_id];
    m_components->hitboxes[m_id].set(transform, radius);

    return *this;
}

EntityComponents& EntityComponents::set_hitbox_size(const float len, const float angle)
{
    const auto transform = m_components->transforms[m_id];
    m_components->hitboxes[m_id].set(transform, len, angle);

    return *this;
}

EntityComponents& EntityComponents::set_hitbox_offset(const RVector2 offset)
{
    m_components->hitboxes[m_id].offset = offset;

    return *this;
}

EntityComponents& EntityComponents::set_sprite_base(const SpriteBase sprite)
{
    m_components->sprites[m_id].base.set(sprite);

    return *this;
}

EntityComponents& EntityComponents::set_sprite_head(const SpriteHead sprite)
{
    m_components->sprites[m_id].head.set(sprite);

    return *this;
}

EntityComponents& EntityComponents::set_sprite_arms(const SpriteArms sprite)
{
    m_components->sprites[m_id].arms.set(sprite);

    return *this;
}

EntityComponents& EntityComponents::set_sprite_legs(const SpriteLegs sprite)
{
    m_components->sprites[m_id].legs.set(sprite);

    return *this;
}

EntityComponents& EntityComponents::set_sprite_extra(const SpriteExtra sprite)
{
    m_components->sprites[m_id].extra.set(sprite);

    return *this;
}

EntityComponents& EntityComponents::set_lifespan(const float lifespan)
{
    m_components->lifespans[m_id] = lifespan;

    return *this;
}

EntityComponents& EntityComponents::set_parent(const unsigned parent)
{
    m_components->parents[m_id] = parent;

    return *this;
}

EntityComponents& EntityComponents::set_hit_damage(const unsigned damage)
{
    m_components->hit_damage[m_id] = damage;

    return *this;
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

bool check_collision(const RRectangle rectangle, const Circle circle)
{
    return rectangle.CheckCollision(circle.pos, circle.radius);
}

bool check_collision(const RRectangle rectangle, const Line line)
{
    const auto rect_pos = rectangle.GetPosition();
    const auto rect_size = rectangle.GetSize();
    const auto rect_line1 = Line(rect_pos, rect_pos + RVector2(rectangle.width, 0.0));
    const auto rect_line2 = Line(rect_pos, rect_pos + RVector2(0.0, rectangle.height));
    const auto rect_line3 = Line(rect_pos + RVector2(rectangle.width, 0.0), rect_pos + rect_size);
    const auto rect_line4 = Line(rect_pos + RVector2(0.0, rectangle.height), rect_pos + rect_size);

    return rectangle.CheckCollision(line.pos1) || rectangle.CheckCollision(line.pos2)
           || check_collision(rect_line1, line) || check_collision(rect_line2, line)
           || check_collision(rect_line3, line) || check_collision(rect_line4, line);
}

bool check_collision(const Circle circle, const RRectangle rectangle)
{
    return check_collision(rectangle, circle);
}

bool check_collision(const Circle circle1, const Circle circle2)
{
    return circle1.radius + circle2.radius > circle1.pos.Distance(circle2.pos);
}

bool check_collision(const Circle circle, const Line line)
{
    return CheckCollisionCircleLine(circle.pos, circle.radius, line.pos1, line.pos2);
}

bool check_collision(const Line line, const RRectangle rectangle)
{
    return check_collision(rectangle, line);
}

bool check_collision(const Line line, const Circle circle)
{
    return check_collision(circle, line);
}

bool check_collision(const Line line1, const Line line2)
{
    return line1.pos1.CheckCollisionLines(line1.pos2, line2.pos1, line2.pos2, nullptr);
}
} // namespace
