#include "sl-math.hpp"

namespace seblib::math
{
Circle::Circle(Vec2 const pos, float const radius)
    : pos{ pos.x + radius, pos.y + radius }
    , radius{ radius }
{
}

auto Circle::draw_lines(rl::Color const color) const -> void
{
    ::DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), radius, color);
}

Line::Line(Vec2 const pos1, Vec2 const pos2)
    : pos1{ pos1 }
    , pos2{ pos2 }
{
}

Line::Line(Vec2 const pos, float const len, float const angle)
    : pos1{ pos }
    , pos2{ pos1 + rl::Vector2{ cos(angle), sin(angle) } * len }
{
}

auto Line::len() const -> float
{
    auto const x_len{ pos2.x - pos1.x };
    auto const y_len{ pos2.y - pos1.y };

    return sqrt((x_len * x_len) + (y_len * y_len));
}

auto Line::draw(rl::Color const color) const -> void
{
    ::DrawLine(
        static_cast<int>(pos1.x), static_cast<int>(pos1.y), static_cast<int>(pos2.x), static_cast<int>(pos2.y), color
    );
}

auto Line::angle() const -> float
{
    return ::Vector2Angle(pos1, pos2);
}

auto check_collision(rl::Rectangle const rectangle1, rl::Rectangle const rectangle2) -> bool
{
    return ::CheckCollisionRecs(rectangle1, rectangle2);
}

auto check_collision(rl::Rectangle const rectangle, Circle const circle) -> bool
{
    return ::CheckCollisionCircleRec(circle.pos, circle.radius, rectangle);
}

auto check_collision(rl::Rectangle const rectangle, Line const line) -> bool
{
    Vec2 const rect_pos{ rectangle.GetPosition() };
    Vec2 const rect_size{ rectangle.GetSize() };
    Line const rect_line1{ rect_pos, rect_pos + Vec2{ rectangle.width, 0.0 } };
    Line const rect_line2{ rect_pos, rect_pos + Vec2{ 0.0, rectangle.height } };
    Line const rect_line3{ rect_pos + Vec2{ rectangle.width, 0.0 }, rect_pos + rect_size };
    Line const rect_line4{ rect_pos + Vec2{ 0.0, rectangle.height }, rect_pos + rect_size };

    return rectangle.CheckCollision(line.pos1)
        || rectangle.CheckCollision(line.pos2)
        || check_collision(rect_line1, line)
        || check_collision(rect_line2, line)
        || check_collision(rect_line3, line)
        || check_collision(rect_line4, line);
}

auto check_collision(Circle const circle, rl::Rectangle const rectangle) -> bool
{
    return check_collision(rectangle, circle);
}

auto check_collision(Circle const circle1, Circle const circle2) -> bool
{
    return circle1.radius + circle2.radius > ::Vector2Distance(circle1.pos, circle2.pos);
}

auto check_collision(Circle const circle, Line const line) -> bool
{
    return ::CheckCollisionCircleLine(circle.pos, circle.radius, line.pos1, line.pos2);
}

auto check_collision(Line const line, rl::Rectangle const rectangle) -> bool
{
    return check_collision(rectangle, line);
}

auto check_collision(Line const line, Circle const circle) -> bool
{
    return check_collision(circle, line);
}

auto check_collision(Line const line1, Line const line2) -> bool
{
    return ::CheckCollisionLines(line1.pos1, line1.pos2, line2.pos1, line2.pos2, nullptr);
}
} // namespace seblib::math
