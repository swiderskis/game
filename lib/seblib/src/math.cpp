#include "seblib-math.hpp"

namespace seblib::math
{
Circle::Circle(const rl::Vector2 pos, const float radius) : pos(pos), radius(radius)
{
}

void Circle::draw_lines(const rl::Color color) const
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

float Line::len() const
{
    const float x_len = pos2.x - pos1.x;
    const float y_len = pos2.y - pos1.y;

    return sqrt((x_len * x_len) + (y_len * y_len));
}

void Line::draw_line(const rl::Color color) const
{
    DrawLine(
        static_cast<int>(pos1.x), static_cast<int>(pos1.y), static_cast<int>(pos2.x), static_cast<int>(pos2.y), color);
}

bool check_collision(const rl::Rectangle rectangle1, const rl::Rectangle rectangle2)
{
    return rectangle1.CheckCollision(rectangle2);
}

bool check_collision(const rl::Rectangle rectangle, const Circle circle)
{
    return rectangle.CheckCollision(circle.pos, circle.radius);
}

bool check_collision(const rl::Rectangle rectangle, const Line line)
{
    const auto rect_pos = rectangle.GetPosition();
    const auto rect_size = rectangle.GetSize();
    const auto rect_line1 = Line(rect_pos, rect_pos + rl::Vector2(rectangle.width, 0.0));
    const auto rect_line2 = Line(rect_pos, rect_pos + rl::Vector2(0.0, rectangle.height));
    const auto rect_line3 = Line(rect_pos + rl::Vector2(rectangle.width, 0.0), rect_pos + rect_size);
    const auto rect_line4 = Line(rect_pos + rl::Vector2(0.0, rectangle.height), rect_pos + rect_size);

    return rectangle.CheckCollision(line.pos1) || rectangle.CheckCollision(line.pos2)
           || check_collision(rect_line1, line) || check_collision(rect_line2, line)
           || check_collision(rect_line3, line) || check_collision(rect_line4, line);
}

bool check_collision(const Circle circle, const rl::Rectangle rectangle)
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

bool check_collision(const Line line, const rl::Rectangle rectangle)
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
} // namespace seblib::math
