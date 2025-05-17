#include "seblib.hpp"

namespace seblib::math
{
Circle::Circle(const RVector2 pos, const float radius) : pos(pos), radius(radius)
{
}

void Circle::draw_lines(const rl::Color color) const
{
    DrawCircleLines((int)pos.x, (int)pos.y, radius, color);
}

Line::Line(const RVector2 pos1, const RVector2 pos2) : pos1(pos1), pos2(pos2)
{
}

Line::Line(const RVector2 pos, const float len, const float angle) : pos1(pos)
{
    pos2 = pos1 + RVector2(len * cos(angle), len * sin(angle));
}

float Line::len() const
{
    const float x_len = pos2.x - pos1.x;
    const float y_len = pos2.y - pos1.y;

    return sqrt((x_len * x_len) + (y_len * y_len));
}

void Line::draw_line(const rl::Color color) const
{
    DrawLine((int)pos1.x, (int)pos1.y, (int)pos2.x, (int)pos2.y, color);
}
} // namespace seblib::math
