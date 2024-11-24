#include "components.hpp"

#include "entities.hpp"
#include "overloaded.hpp"

#include <cassert>
#include <cstddef>

constexpr int RECTANGLE_BBOX_INDEX = 0;

void Sprite::set_sprite(const SpriteType sprite_type)
{
    assert(sprite_type != SpriteType::None);

    if (type == sprite_type) {
        return;
    }

    type = sprite_type;
    current_frame = 0;
    frame_update_dt = 0.0;
}

void Sprite::check_update_frame(const float dt)
{
    const auto details = DETAILS[(size_t)type];
    if (details.frames == 1) {
        return;
    }

    frame_update_dt += dt;
    if (frame_update_dt < details.frame_duration) {
        return;
    }

    frame_update_dt = 0.0;
    current_frame += 1;
    if (current_frame == details.frames) {
        current_frame = 0;
    }
}

void Sprite::lookup_set_idle_sprite(Entity entity)
{
    auto idle_sprite = SpriteType::None;
    switch (entity) {
    case Entity::Player:
        idle_sprite = SpriteType::PlayerIdle;
        break;
    default:
        return;
    }

    set_sprite(idle_sprite);
}

void Sprite::lookup_set_walk_sprite(Entity entity)
{
    auto walk_sprite = SpriteType::None;
    switch (entity) {
    case Entity::Player:
        walk_sprite = SpriteType::PlayerWalk;
        break;
    default:
        return;
    }

    set_sprite(walk_sprite);
}

RRectangle Sprite::sprite() const
{
    const auto details = DETAILS[(size_t)type];
    const auto pos = RVector2(details.x + (details.size * (float)current_frame), details.y);
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
               bounding_box);
}

bool BBox::collides(const BBox other_bounding_box) const
{
    const auto rect_bbox = [other_bounding_box](RRectangle bbox) {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return bbox.CheckCollision(other_bbox); },
                [bbox](const Circle other_bbox) { return bbox.CheckCollision(other_bbox.pos, other_bbox.radius); },
            },
            other_bounding_box.bounding_box);
    };
    const auto circle_bbox = [other_bounding_box](Circle bbox) {
        return std::visit(
            overloaded{
                [bbox](const RRectangle other_bbox) { return other_bbox.CheckCollision(bbox.pos, bbox.radius); },
                [bbox](const Circle other_bbox) { return bbox.check_collision(other_bbox); },
            },
            other_bounding_box.bounding_box);
    };

    return std::visit(
        overloaded{
            rect_bbox,
            circle_bbox,
        },
        bounding_box);
}

bool BBox::x_overlaps(const BBox other_bounding_box) const
{
    assert(bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(bounding_box);

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
                      other_bounding_box.bounding_box);
}

bool BBox::y_overlaps(const BBox other_bounding_box) const
{
    assert(bounding_box.index() == RECTANGLE_BBOX_INDEX);

    const auto bbox = std::get<RRectangle>(bounding_box);

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
                      other_bounding_box.bounding_box);
}

void BBox::set_size(const RVector2 size)
{
    bounding_box = RRectangle(RVector2(0.0, 0.0), size);
}

void BBox::set_size(const float radius)
{
    bounding_box = Circle(RVector2(0.0, 0.0), radius);
}

BBox::BBox(const RVector2 pos, const float radius) : bounding_box(Circle(pos, radius))
{
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
