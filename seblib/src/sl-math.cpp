#include "sl-math.hpp"

namespace seblib::math
{
Circle::Circle(const rl::Vector2 pos, const float radius) : pos{ pos.x + radius, pos.y + radius }, radius(radius)
{
}

auto Circle::draw_lines(const rl::Color color) const -> void
{
    DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), radius, color);
}

Line::Line(const rl::Vector2 pos1, const rl::Vector2 pos2) : pos1(pos1), pos2(pos2)
{
}

Line::Line(const rl::Vector2 pos, const float len, const float angle) : pos1(pos)
{
    pos2 = pos1 + rl::Vector2(len * cos(angle), len * sin(angle));
}

auto Line::len() const -> float
{
    const float x_len{ pos2.x - pos1.x };
    const float y_len{ pos2.y - pos1.y };

    return sqrt((x_len * x_len) + (y_len * y_len));
}

auto Line::draw_line(const rl::Color color) const -> void
{
    DrawLine(
        static_cast<int>(pos1.x), static_cast<int>(pos1.y), static_cast<int>(pos2.x), static_cast<int>(pos2.y), color);
}

auto check_collision(const rl::Rectangle rectangle1, const rl::Rectangle rectangle2) -> bool
{
    return rectangle1.CheckCollision(rectangle2);
}

auto check_collision(const rl::Rectangle rectangle, const Circle circle) -> bool
{
    return rectangle.CheckCollision(circle.pos, circle.radius);
}

auto check_collision(const rl::Rectangle rectangle, const Line line) -> bool
{
    const auto rect_pos{ rectangle.GetPosition() };
    const auto rect_size{ rectangle.GetSize() };
    const Line rect_line1{ rect_pos, rect_pos + rl::Vector2(rectangle.width, 0.0) };
    const Line rect_line2{ rect_pos, rect_pos + rl::Vector2(0.0, rectangle.height) };
    const Line rect_line3{ rect_pos + rl::Vector2(rectangle.width, 0.0), rect_pos + rect_size };
    const Line rect_line4{ rect_pos + rl::Vector2(0.0, rectangle.height), rect_pos + rect_size };

    return rectangle.CheckCollision(line.pos1) || rectangle.CheckCollision(line.pos2)
           || check_collision(rect_line1, line) || check_collision(rect_line2, line)
           || check_collision(rect_line3, line) || check_collision(rect_line4, line);
}

auto check_collision(const Circle circle, const rl::Rectangle rectangle) -> bool
{
    return check_collision(rectangle, circle);
}

auto check_collision(const Circle circle1, const Circle circle2) -> bool
{
    return circle1.radius + circle2.radius > circle1.pos.Distance(circle2.pos);
}

auto check_collision(const Circle circle, const Line line) -> bool
{
    return CheckCollisionCircleLine(circle.pos, circle.radius, line.pos1, line.pos2);
}

auto check_collision(const Line line, const rl::Rectangle rectangle) -> bool
{
    return check_collision(rectangle, line);
}

auto check_collision(const Line line, const Circle circle) -> bool
{
    return check_collision(circle, line);
}

auto check_collision(const Line line1, const Line line2) -> bool
{
    return line1.pos1.CheckCollisionLines(line1.pos2, line2.pos1, line2.pos2, nullptr);
}
} // namespace seblib::math
