#include "components.hpp"

#include "overloaded.hpp"

#include <cassert>

constexpr int RECTANGLE_BBOX_INDEX = 0;
constexpr int CIRCLE_BBOX_INDEX = 1;

void Tform::move(float dt)
{
    pos += vel * dt;
}

RVector2 Sprite::size() const
{
    return sprite.GetSize();
}

void Sprite::set_pos(RVector2 pos)
{
    sprite.SetPosition(pos);
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

bool Circle::check_collision(Circle other_circle) const
{
    return radius + other_circle.radius > pos.Distance(other_circle.pos);
}

void Circle::draw_lines(::Color color)
{
    DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), radius, color);
}

Circle::Circle(RVector2 pos, float radius) : pos(pos), radius(radius) {};

void BBox::sync(Tform transform)
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

bool BBox::collides(BBox other_bounding_box) const
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
};

bool BBox::x_overlaps(BBox other_bounding_box) const
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

bool BBox::y_overlaps(BBox other_bounding_box) const
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

void BBox::set_size(RVector2 size)
{
    assert(bounding_box.index() == RECTANGLE_BBOX_INDEX);

    auto& bbox = std::get<RRectangle>(bounding_box);
    bbox.SetSize(size);
}

void BBox::set_size(float radius)
{
    assert(bounding_box.index() == CIRCLE_BBOX_INDEX);

    auto& bbox = std::get<Circle>(bounding_box);
    bbox.radius = radius;
}

BBox::BBox(RVector2 pos, float radius) : bounding_box(Circle(pos, radius))
{
}

void Health::set_health(int health)
{
    current = health;
    max = health;
}

float Health::percentage() const
{
    const auto current_health = static_cast<float>(current);
    const auto max_health = static_cast<float>(max.value());

    return current_health / max_health;
}
