#include "components.hpp"

#include "overloaded.hpp"

#include <cassert>
#include <cstddef>

constexpr int RECTANGLE_BBOX_INDEX = 0;
constexpr int CIRCLE_BBOX_INDEX = 1;

void Sprite::set_sprite(const SpriteType type)
{
    sprite.SetPosition(RVector2(SHEET_POS[(size_t)type].x, SHEET_POS[(size_t)type].y));
}

void Sprite::flip()
{
    auto size = sprite.GetSize();
    if (size.x < 0) {
        return;
    }

    size.x *= -1;
    sprite.SetSize(size);
}

void Sprite::unflip()
{
    auto size = sprite.GetSize();
    if (size.x > 0) {
        return;
    }

    size.x *= -1;
    sprite.SetSize(size);
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
                [bbox](RRectangle other_bbox) { return bbox.CheckCollision(other_bbox); },
                [bbox](Circle other_bbox) { return bbox.CheckCollision(other_bbox.pos, other_bbox.radius); },
            },
            other_bounding_box.bounding_box);
    };
    const auto circle_bbox = [other_bounding_box](Circle bbox) {
        return std::visit(
            overloaded{
                [bbox](RRectangle other_bbox) { return other_bbox.CheckCollision(bbox.pos, bbox.radius); },
                [bbox](Circle other_bbox) { return bbox.check_collision(other_bbox); },
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
                          [bbox](RRectangle other_bbox) {
                              return (bbox.x >= other_bbox.x && bbox.x - other_bbox.x < other_bbox.width)
                                     || (other_bbox.x >= bbox.x && other_bbox.x - bbox.x < bbox.width);
                          },
                          [bbox](Circle other_bbox) {
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
                          [bbox](RRectangle other_bbox) {
                              return (bbox.y >= other_bbox.y && bbox.y - other_bbox.y < other_bbox.height)
                                     || (other_bbox.y >= bbox.y && other_bbox.y - bbox.y < bbox.height);
                          },
                          [bbox](Circle other_bbox) {
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
