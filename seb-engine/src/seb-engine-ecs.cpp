#include "seb-engine-ecs.hpp"

#include "seblib-log.hpp"
#include "seblib-math.hpp"
#include "seblib.hpp"

#include <algorithm>
#include <cassert>

namespace seb_engine
{
namespace sl = seblib;

Components::Components()
{
    reg<Pos>();
    reg<Vel>();
    reg<BBox>();
}

void Components::uninit_destroyed_entity(const unsigned id)
{
    for (auto& [_, component] : m_components)
    {
        component->reset(id);
    }
}

void Components::move(const float dt)
{
    auto& pos = vec<Pos>();
    auto& vel = vec<Vel>();
    std::ranges::transform(pos, vel, pos.begin(), [dt](const auto pos, const auto vel) { return pos + (vel * dt); });
}

EntityComponents Components::by_id(const unsigned id)
{
    return { *this, id };
}

EntityComponents::EntityComponents(Components& components, const unsigned id) : m_components(&components), m_id(id)
{
}

BBox::BBox(const BBoxVariant bbox) : BBox{ bbox, rl::Vector2{} }
{
}

BBox::BBox(const BBoxVariant bbox, const rl::Vector2 offset) : m_bbox{ bbox }, m_offset{ offset }
{
    sl::match(
        m_bbox,
        [this](rl::Rectangle bbox) { sync(rl::Vector2{ bbox.x, bbox.y }); },
        [this](sm::Circle bbox) { sync(rl::Vector2{ bbox.pos.x, bbox.pos.y }); },
        [this](sm::Line bbox) { sync(rl::Vector2{ bbox.pos1.x, bbox.pos1.y }); });
}

void BBox::sync(rl::Vector2 pos)
{
    sl::match(
        m_bbox,
        [pos, this](rl::Rectangle& bbox)
        {
            bbox.SetPosition(pos);
            bbox.x += m_offset.x;
            bbox.y += m_offset.y;
            slog::log(
                slog::TRC, "Rectangle bbox pos: ({}, {}), size: ({}, {})", bbox.x, bbox.y, bbox.width, bbox.height);
        },
        [pos, this](sm::Circle& bbox)
        {
            bbox.pos = pos;
            bbox.pos.x += bbox.radius;
            bbox.pos.y += bbox.radius;
            bbox.pos.x += m_offset.x;
            bbox.pos.y += m_offset.y;
            slog::log(slog::TRC, "Circle bbox pos: ({}, {}), radius: {}", bbox.pos.x, bbox.pos.y, bbox.radius);
        },
        [pos, this](sm::Line& bbox)
        {
            bbox.pos2 = pos + (bbox.pos2 - bbox.pos1);
            bbox.pos1 = pos;
            bbox.pos1.x += m_offset.x;
            bbox.pos1.y += m_offset.y;
            bbox.pos2.x += m_offset.x;
            bbox.pos2.y += m_offset.y;
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

BBoxVariant BBox::val() const
{
    return m_bbox;
}
} // namespace seb_engine
