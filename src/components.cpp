#include "components.hpp"

#include "overloaded.hpp"

#include <cassert>
#include <cstddef>

// NOLINTBEGIN(*avoid-c-arrays)
static constexpr struct {
    float x;
    float y;
    float size;
    unsigned frames;
    float frame_duration;
} SPRITE_DETAILS[] = {
    [(size_t)SpriteType::PlayerIdle] = { 128.0, 0.0, TILE_SIZE, 2, 0.5 },
    [(size_t)SpriteType::PlayerWalk] = { 128.0, 32.0, TILE_SIZE, 4, 0.16 },
    [(size_t)SpriteType::PlayerJump] = { 128.0, 64.0, TILE_SIZE, 1, 0.0 },
    [(size_t)SpriteType::PlayerFall] = { 128.0, 96.0, TILE_SIZE, 4, 0.1 },
    [(size_t)SpriteType::Projectile] = { 128.0, 128.0, TILE_SIZE, 1, 0.0 },
    [(size_t)SpriteType::Enemy] = { 128.0, 160.0, TILE_SIZE, 1, 0.0 },

    // tiles
    [(size_t)SpriteType::TileBrick] = { 0.0, 0.0, TILE_SIZE, 1, 0.0 },
};

// NOLINTEND(*avoid-c-arrays)

inline constexpr int RECTANGLE_BBOX_INDEX = 0;

std::optional<SpriteType> lookup_idle_sprite(Entity entity);
std::optional<SpriteType> lookup_walk_sprite(Entity entity);
std::optional<SpriteType> lookup_jump_sprite(Entity entity);
std::optional<SpriteType> lookup_fall_sprite(Entity entity);

void Sprite::set(const SpriteType sprite_type)
{
    if (m_type == sprite_type) {
        return;
    }

    m_type = sprite_type;
    m_current_frame = 0;
    m_frame_update_dt = 0.0;
}

void Sprite::check_update_frame(const float dt)
{
    const auto details = SPRITE_DETAILS[(size_t)m_type];
    if (details.frames == 1) {
        return;
    }

    m_frame_update_dt += dt;
    if (m_frame_update_dt < details.frame_duration) {
        return;
    }

    m_frame_update_dt = 0.0;
    m_current_frame += 1;
    if (m_current_frame == details.frames) {
        m_current_frame = 0;
    }
}

RRectangle Sprite::sprite() const
{
    const auto details = SPRITE_DETAILS[(size_t)m_type];
    const auto pos = RVector2(details.x + (details.size * (float)m_current_frame), details.y);
    const auto size = RVector2(details.size * (float)(flipped ? -1 : 1), details.size);

    return { pos, size };
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
    std::visit(overloaded{
                   [transform](RRectangle& bbox) {
                       bbox.SetPosition(transform.pos);
                       bbox.x += (TILE_SIZE - bbox.width) / 2;
                       bbox.y += (TILE_SIZE - bbox.height);
                   },
                   [transform](Circle& bbox) {
                       bbox.pos = transform.pos;
                       bbox.pos.x += TILE_SIZE / 2;
                       bbox.pos.y += TILE_SIZE / 2;
                   },
               },
               m_bounding_box);
}

bool BBox::collides(const BBox other_bounding_box) const
{
    const auto rect_bbox = [other_bounding_box](RRectangle bbox) {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return bbox.CheckCollision(other_bbox); },
                [bbox](const Circle other_bbox) { return bbox.CheckCollision(other_bbox.pos, other_bbox.radius); },
            },
            other_bounding_box.m_bounding_box);
    };
    const auto circle_bbox = [other_bounding_box](Circle bbox) {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return other_bbox.CheckCollision(bbox.pos, bbox.radius); },
                [bbox](const Circle other_bbox) { return bbox.check_collision(other_bbox); },
            },
            other_bounding_box.m_bounding_box);
    };

    return std::visit(
        overloaded{
            rect_bbox,
            circle_bbox,
        },
        m_bounding_box);
}

bool BBox::x_overlaps(const BBox other_bounding_box) const
{
    assert(m_bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return std::visit(overloaded{
                          [bbox](const RRectangle other_bbox) {
                              return (bbox.x >= other_bbox.x && bbox.x - other_bbox.x < other_bbox.width)
                                     || (other_bbox.x >= bbox.x && other_bbox.x - bbox.x < bbox.width);
                          },
                          [bbox](const Circle other_bbox) {
                              return (bbox.x >= other_bbox.pos.x && other_bbox.pos.x + other_bbox.radius > bbox.x)
                                     || (other_bbox.pos.x >= bbox.x
                                         && bbox.x + bbox.width > other_bbox.pos.x - other_bbox.radius);
                          },
                      },
                      other_bounding_box.m_bounding_box);
}

bool BBox::y_overlaps(const BBox other_bounding_box) const
{
    assert(m_bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(m_bounding_box);

    return std::visit(overloaded{
                          [bbox](const RRectangle other_bbox) {
                              return (bbox.y >= other_bbox.y && bbox.y - other_bbox.y < other_bbox.height)
                                     || (other_bbox.y >= bbox.y && other_bbox.y - bbox.y < bbox.height);
                          },
                          [bbox](const Circle other_bbox) {
                              return (bbox.y >= other_bbox.pos.y && other_bbox.pos.y + other_bbox.radius > bbox.y)
                                     || (other_bbox.pos.y >= bbox.y
                                         && bbox.y + bbox.height > other_bbox.pos.y - other_bbox.radius);
                          },
                      },
                      other_bounding_box.m_bounding_box);
}

void BBox::set(Tform transform, RVector2 size)
{
    m_bounding_box = RRectangle(RVector2(0.0, 0.0), size);
    sync(transform);
}

void BBox::set(Tform transform, float radius)
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
    const auto current_health = (float)current;
    const auto max_health = (float)max.value();

    return current_health / max_health;
}

std::optional<SpriteType> components::lookup_movement_sprite(const Entity entity, const RVector2 vel)
{
    if (vel.y < 0) {
        return lookup_jump_sprite(entity);
    }

    if (vel.y > 0) {
        return lookup_fall_sprite(entity);
    }

    if (vel.x != 0) {
        return lookup_walk_sprite(entity);
    }

    return lookup_idle_sprite(entity);
}

std::optional<SpriteType> lookup_idle_sprite(const Entity entity)
{
    switch (entity) {
    case Entity::Player:
        return SpriteType::PlayerIdle;
    default:
        return std::nullopt;
    }
}

std::optional<SpriteType> lookup_walk_sprite(const Entity entity)
{
    switch (entity) {
    case Entity::Player:
        return SpriteType::PlayerWalk;
    default:
        return std::nullopt;
    }
}

std::optional<SpriteType> lookup_jump_sprite(const Entity entity)
{
    switch (entity) {
    case Entity::Player:
        return SpriteType::PlayerJump;
    default:
        return std::nullopt;
    }
}

std::optional<SpriteType> lookup_fall_sprite(const Entity entity)
{
    switch (entity) {
    case Entity::Player:
        return SpriteType::PlayerFall;
    default:
        return std::nullopt;
    }
}
