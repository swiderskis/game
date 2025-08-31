#ifndef SL_MATH_HPP_
#define SL_MATH_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <numbers>

namespace seblib::math
{
namespace rl = raylib;

struct Circle
{
    rl::Vector2 pos;
    float radius = 0.0;

    Circle(rl::Vector2 pos, float radius);

    void draw_lines(rl::Color color) const;
};

struct Line
{
    rl::Vector2 pos1;
    rl::Vector2 pos2;

    Line() = delete;
    Line(rl::Vector2 pos1, rl::Vector2 pos2);
    Line(rl::Vector2 pos, float len, float angle);

    [[nodiscard]] float len() const;
    void draw_line(rl::Color color) const;
};

constexpr float degrees_to_radians(float ang);
constexpr float radians_to_degrees(float ang);
bool check_collision(rl::Rectangle rectangle1, rl::Rectangle rectangle2);
bool check_collision(rl::Rectangle rectangle, Circle circle);
bool check_collision(rl::Rectangle rectangle, Line line);
bool check_collision(Circle circle, rl::Rectangle rectangle);
bool check_collision(Circle circle1, Circle circle2);
bool check_collision(Circle circle, Line line);
bool check_collision(Line line, rl::Rectangle rectangle);
bool check_collision(Line line, Circle circle);
bool check_collision(Line line1, Line line2);
} // namespace seblib::math

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seblib::math
{
constexpr float degrees_to_radians(const float ang)
{
    return static_cast<float>(ang * std::numbers::pi / 180.0); // NOLINT(*magic-numbers)
}

constexpr float radians_to_degrees(const float ang)
{
    return static_cast<float>(ang * 180.0 / std::numbers::pi); // NOLINT(*magic-numbers)
}
} // namespace seblib::math

#endif
