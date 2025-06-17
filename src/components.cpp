#include "components.hpp"

#include "seb-engine-sprite.hpp"
#include "seblib.hpp"

#include <cassert>
#include <cmath>
#include <optional>

namespace rl = raylib;
namespace sl = seblib;
namespace slog = seblib::log;
namespace sm = seblib::math;
namespace se = seb_engine;

void BBox::sync(const rl::Vector2 pos, const bool flipped)
{
    sl::match(
        m_bbox,
        [pos, this, flipped](rl::Rectangle& bbox)
        {
            bbox.SetPosition(pos);
            bbox.x += (se::SPRITE_SIZE - bbox.width) / 2;
            bbox.y += (se::SPRITE_SIZE - bbox.height);
            bbox.x += m_offset.x * (flipped ? -1.0F : 1.0F);
            bbox.y -= m_offset.y;
            slog::log(slog::TRC, "Rectangle bbox pos: ({}, {})", bbox.x, bbox.y);
        },
        [pos, this, flipped](sm::Circle& bbox)
        {
            bbox.pos = pos;
            bbox.pos.x += se::SPRITE_SIZE / 2;
            bbox.pos.y += se::SPRITE_SIZE / 2;
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
            bbox.pos1.x += se::SPRITE_SIZE / 2;
            bbox.pos1.y += se::SPRITE_SIZE / 2;
            bbox.pos2.x += se::SPRITE_SIZE / 2;
            bbox.pos2.y += se::SPRITE_SIZE / 2;
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
                                          [bbox](const auto other_bbox)
                                          { return sm::check_collision(bbox, other_bbox); });
                     });
}

bool BBox::x_overlaps(const BBox other_bbox) const
{
    // method currently should only be used for tile collision correction, tiles always have rectangular bboxes
    assert(m_bbox.index() == RECTANGLE);

    const auto bbox = std::get<rl::Rectangle>(m_bbox);

    return sl::match(
        other_bbox.m_bbox,
        [bbox](const rl::Rectangle other_bbox)
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

    const auto bbox = std::get<rl::Rectangle>(m_bbox);

    return sl::match(
        other_bbox.m_bbox,
        [bbox](const rl::Rectangle other_bbox)
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

void BBox::set(const rl::Vector2 pos, const rl::Vector2 size)
{
    m_bbox = rl::Rectangle(rl::Vector2(0.0, 0.0), size);
    sync(pos, false);
}

void BBox::set(const rl::Vector2 pos, const float radius)
{
    m_bbox = sm::Circle(rl::Vector2(0.0, 0.0), radius);
    sync(pos, false);
}

void BBox::set(const rl::Vector2 pos, const float len, const float angle)
{
    m_bbox = sm::Line(rl::Vector2(0.0, 0.0), len, angle);
    sync(pos, false);
}

void BBox::set_offset(rl::Vector2 pos, rl::Vector2 offset)
{
    m_offset = offset;
    sync(pos, false);
}

BBoxVariant BBox::bbox() const
{
    return m_bbox;
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
