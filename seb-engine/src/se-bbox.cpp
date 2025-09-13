#include "se-bbox.hpp"

#include "seblib.hpp"
#include "sl-log.hpp"
#include "sl-math.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace seb_engine::bbox;

namespace rl = raylib;
namespace slog = seblib::log;
namespace sm = seblib::math;

namespace
{
auto resolve_collision(rl::Rectangle bbox1, rl::Rectangle bbox2) -> rl::Vector2;
auto resolve_collision(rl::Rectangle bbox1, sm::Circle bbox2) -> rl::Vector2;
auto resolve_collision(rl::Rectangle bbox1, sm::Line bbox2) -> rl::Vector2;
auto resolve_collision(sm::Circle bbox1, rl::Rectangle bbox2) -> rl::Vector2;
auto resolve_collision(sm::Circle bbox1, sm::Circle bbox2) -> rl::Vector2;
auto resolve_collision(sm::Circle bbox1, sm::Line bbox2) -> rl::Vector2;
auto resolve_collision(sm::Line bbox1, rl::Rectangle bbox2) -> rl::Vector2;
auto resolve_collision(sm::Line bbox1, sm::Circle bbox2) -> rl::Vector2;
auto resolve_collision(sm::Line bbox1, sm::Line bbox2) -> rl::Vector2;
} // namespace

namespace seb_engine
{
BBox::BBox(const BBoxDetails bbox)
    : BBox{ bbox, rl::Vector2{} }
{
}

BBox::BBox(const BBoxDetails bbox, const rl::Vector2 offset)
    : m_bbox{ bbox }
    , m_offset{ offset }
{
}

auto BBox::val(rl::Vector2 pos) const -> BBoxVariant
{
    return sl::match(
        m_bbox,
        [pos, this](const BBoxRect bbox) -> BBoxVariant { return rl::Rectangle{ pos + m_offset, bbox.size }; },
        [pos, this](const BBoxCircle bbox) -> BBoxVariant { return sm::Circle{ pos + m_offset, bbox.radius }; },
        [pos, this](const BBoxLine bbox) -> BBoxVariant { return sm::Line{ pos + m_offset, bbox.len, bbox.angle }; }
    );
}

auto BBox::details() const -> BBoxDetails
{
    return m_bbox;
}

namespace bbox
{
auto collides(const BBoxVariant bbox1, const BBoxVariant bbox2) -> bool
{
    return sl::match(
        bbox1,
        [bbox2](const auto bbox1)
        { return sl::match(bbox2, [bbox1](const auto bbox2) { return sm::check_collision(bbox1, bbox2); }); }
    );
}

// currently assumes bbox2 is unmoving and unmovable, return value only resolves bbox1 pos
auto resolve_collision(const BBoxVariant bbox1, const BBoxVariant bbox2) -> rl::Vector2
{
    return sl::match(
        bbox1,
        [bbox2](const auto bbox1)
        { return sl::match(bbox2, [bbox1](const auto bbox2) { return ::resolve_collision(bbox1, bbox2); }); }
    );
}
} // namespace bbox
} // namespace seb_engine

namespace
{
auto resolve_collision(const rl::Rectangle bbox1, const rl::Rectangle bbox2) -> rl::Vector2
{
    const auto x_overlap{ (bbox1.x > bbox2.x ? bbox2.x + bbox2.width - bbox1.x : bbox2.x - (bbox1.x + bbox1.width)) };
    const auto y_overlap{ (bbox1.y > bbox2.y ? bbox2.y + bbox2.height - bbox1.y : bbox2.y - (bbox1.y + bbox1.height)) };

    return (
        std::fabs(y_overlap) > std::fabs(x_overlap) ? rl::Vector2{ x_overlap, 0.0 } : rl::Vector2{ 0.0, y_overlap }
    );
}

auto resolve_collision(const rl::Rectangle bbox1, const sm::Circle bbox2) -> rl::Vector2
{
    const rl::Vector2 corner{ (bbox2.pos.x > bbox1.x ? bbox1.x + bbox1.width : bbox1.x),
                              (bbox2.pos.y > bbox1.y ? bbox1.y + bbox1.height : bbox1.y) };
    const auto y_adjust_only{ bbox2.pos.x > bbox1.x && bbox2.pos.x < bbox1.x + bbox1.width };
    const auto x_adjust_only{ bbox2.pos.y > bbox1.y && bbox2.pos.y < bbox1.y + bbox1.height };
    const rl::Vector2 diff{ (y_adjust_only ? 0.0F : bbox2.pos.x - corner.x),
                            (x_adjust_only ? 0.0F : bbox2.pos.y - corner.y) };
    const auto angle{ std::atan2(diff.y, diff.x) };
    slog::log(slog::TRC, "Angle {}", sm::radians_to_degrees(angle));

    return { diff - rl::Vector2{ cos(angle), sin(angle) } * bbox2.radius };
}

auto resolve_collision(const rl::Rectangle bbox1, const sm::Line bbox2) -> rl::Vector2
{
    const auto pos1{ bbox2.pos1 };
    const auto pos2{ bbox2.pos2 };
    const auto pos1_in_rect{ pos1.x >= bbox1.x && pos1.x <= bbox1.x + bbox1.width && pos1.y >= bbox1.y
                             && pos1.y <= bbox1.y + bbox1.height };
    const auto pos2_in_rect{ pos2.x >= bbox1.x && pos2.x <= bbox1.x + bbox1.width && pos2.y >= bbox1.y
                             && pos2.y <= bbox1.y + bbox1.height };
    // collision resolution with one of line ends
    if (pos1_in_rect ^ pos2_in_rect)
    {
        const auto x_adjust{
            (pos1_in_rect ? pos1.x - (pos1.x > pos2.x ? bbox1.x : bbox1.x + bbox1.width)
                          : pos2.x - (pos2.x > pos1.x ? bbox1.x : bbox1.x + bbox1.width))
        };
        const auto y_adjust{
            (pos1_in_rect ? pos1.y - (pos1.y > pos2.y ? bbox1.y : bbox1.y + bbox1.height)
                          : pos2.y - (pos2.y > pos1.y ? bbox1.y : bbox1.y + bbox1.height))
        };

        return (
            std::fabs(x_adjust) > std::fabs(y_adjust) ? rl::Vector2{ 0.0, y_adjust } : rl::Vector2{ x_adjust, 0.0 }
        );
    }

    const auto upward_line{ (pos2.x > pos1.x && pos1.y > pos2.y) || (pos1.x > pos2.x && pos2.y > pos1.y) };
    const rl::Vector2 corner1{ bbox1.x, (upward_line ? bbox1.y : bbox1.y + bbox1.height) };
    const rl::Vector2 corner2{ bbox1.x + bbox1.width, (upward_line ? bbox1.y + bbox1.height : bbox1.y) };
    const auto diff{ pos1 - pos2 };
    const auto closest_point1{ diff * ::Vector2DotProduct(corner1 - pos1, diff) / diff.LengthSqr() };
    const auto closest_point2{ diff * ::Vector2DotProduct(corner2 - pos1, diff) / diff.LengthSqr() };
    const auto adjust1{ pos1 + closest_point1 - corner1 };
    const auto adjust2{ pos1 + closest_point2 - corner2 };

    return (adjust1.Length() > adjust2.Length() ? adjust2 : adjust1);
}

auto resolve_collision(const sm::Circle bbox1, const rl::Rectangle bbox2) -> rl::Vector2
{
    return -resolve_collision(bbox2, bbox1);
}

auto resolve_collision(const sm::Circle bbox1, const sm::Circle bbox2) -> rl::Vector2
{
    const auto distance{ bbox1.pos - bbox2.pos };
    const auto overlap{ bbox1.radius + bbox2.radius - distance.Length() };

    return distance * overlap / distance.Length();
}

auto resolve_collision(const sm::Circle bbox1, const sm::Line bbox2) -> rl::Vector2
{
    const auto pos1{ bbox2.pos1 };
    const auto pos2{ bbox2.pos2 };
    const auto diff{ pos2 - pos1 };
    const auto closest_line_point{
        diff * std::clamp(::Vector2DotProduct(bbox1.pos - pos1, diff) / diff.LengthSqr(), 0.0F, 1.0F)
    };
    const auto point_to_line{ pos1 + closest_line_point - bbox1.pos };

    return -point_to_line * ((bbox1.radius - point_to_line.Length()) / point_to_line.Length());
}

auto resolve_collision(const sm::Line bbox1, const rl::Rectangle bbox2) -> rl::Vector2
{
    return -resolve_collision(bbox2, bbox1);
}

auto resolve_collision(const sm::Line bbox1, const sm::Circle bbox2) -> rl::Vector2
{
    return -resolve_collision(bbox2, bbox1);
}

auto resolve_collision(const sm::Line bbox1, const sm::Line bbox2) -> rl::Vector2
{
    rl::Vector2 collision_point;
    ::CheckCollisionLines(bbox1.pos1, bbox1.pos2, bbox2.pos1, bbox2.pos2, &collision_point);
    const auto mid_adjust1{ collision_point - bbox1.pos1 };
    const auto mid_adjust2{ collision_point - bbox1.pos2 };
    const auto end_adjust1{ bbox2.pos1 - collision_point };
    const auto end_adjust2{ bbox2.pos2 - collision_point };
    const auto mid_adjust{ mid_adjust1.Length() > mid_adjust2.Length() ? mid_adjust2 : mid_adjust1 };
    const auto end_adjust{ end_adjust1.Length() > end_adjust2.Length() ? end_adjust2 : end_adjust1 };

    return (mid_adjust.Length() > end_adjust.Length() ? end_adjust : mid_adjust);
}
} // namespace
