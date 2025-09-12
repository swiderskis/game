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
    float radius{ 0.0 };

    Circle(rl::Vector2 pos, float radius);

    auto draw_lines(rl::Color color) const -> void;
};

struct Line
{
    rl::Vector2 pos1;
    rl::Vector2 pos2;

    Line() = delete;
    Line(rl::Vector2 pos1, rl::Vector2 pos2);
    Line(rl::Vector2 pos, float len, float angle);

    [[nodiscard]] auto len() const -> float;
    auto draw(rl::Color color) const -> void;
    [[nodiscard]] auto angle() const -> float;
};

constexpr auto degrees_to_radians(float ang) -> float;
constexpr auto radians_to_degrees(float ang) -> float;
auto check_collision(rl::Rectangle rectangle1, rl::Rectangle rectangle2) -> bool;
auto check_collision(rl::Rectangle rectangle, Circle circle) -> bool;
auto check_collision(rl::Rectangle rectangle, Line line) -> bool;
auto check_collision(Circle circle, rl::Rectangle rectangle) -> bool;
auto check_collision(Circle circle1, Circle circle2) -> bool;
auto check_collision(Circle circle, Line line) -> bool;
auto check_collision(Line line, rl::Rectangle rectangle) -> bool;
auto check_collision(Line line, Circle circle) -> bool;
auto check_collision(Line line1, Line line2) -> bool;
} // namespace seblib::math

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seblib::math
{
constexpr auto degrees_to_radians(const float ang) -> float
{
    return static_cast<float>(ang * std::numbers::pi / 180.0); // NOLINT(*magic-numbers)
}

constexpr auto radians_to_degrees(const float ang) -> float
{
    return static_cast<float>(ang * 180.0 / std::numbers::pi); // NOLINT(*magic-numbers)
}
} // namespace seblib::math

#endif
