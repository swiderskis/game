#ifndef SL_MATH_HPP_
#define SL_MATH_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-log.hpp"

#include <cmath>
#include <numbers>

namespace seblib::math
{
namespace rl = raylib;
namespace slog = seblib::log;

template <typename P>
struct Point;

struct V2;
using Vec2 = Point<V2>;

template <typename P>
struct Point
{
    float x{ 0.0 };
    float y{ 0.0 };

    Point() = default;
    constexpr Point(float x, float y);
    explicit constexpr Point(rl::Vector2 vec);

    operator rl::Vector2() const; // NOLINT(hicpp-explicit-conversions)
    [[maybe_unused]] auto operator=(rl::Vector2 vec) -> Point&;
    [[nodiscard]] auto operator+(rl::Vector2 vec) const -> Point;
    [[nodiscard]] auto operator-(rl::Vector2 vec) const -> Point;
    [[nodiscard]] auto operator*(float f) const -> Point;
    [[nodiscard]] auto operator/(float f) const -> Point;
    [[maybe_unused]] auto operator+=(rl::Vector2 vec) -> Point&;
    [[maybe_unused]] auto operator-=(rl::Vector2 vec) -> Point&;
    [[maybe_unused]] auto operator*=(float f) -> Point&;
    [[maybe_unused]] auto operator/=(float f) -> Point&;
    [[nodiscard]] auto len() const -> float;
    [[nodiscard]] auto sqr_len() const -> float;
    [[nodiscard]] auto operator-() const -> Point;
    operator Vec2() const; // NOLINT(hicpp-explicit-conversions)
    [[nodiscard]] auto operator+(Point vec) const -> Point;
    [[nodiscard]] auto operator-(Point vec) const -> Point;
    [[nodiscard]] auto operator*(Point vec) const -> Point;
    [[nodiscard]] auto operator/(Point vec) const -> Point;
    [[maybe_unused]] auto operator+=(Point vec) -> Point&;
    [[maybe_unused]] auto operator-=(Point vec) -> Point&;
    [[maybe_unused]] auto operator*=(Point vec) -> Point&;
    [[maybe_unused]] auto operator/=(Point vec) -> Point&;
};

struct Circle
{
    Vec2 pos;
    float radius{ 0.0 };

    Circle(Vec2 pos, float radius);

    auto draw_lines(rl::Color color) const -> void;
};

struct Line
{
    Vec2 pos1;
    Vec2 pos2;

    Line() = delete;
    Line(Vec2 pos1, Vec2 pos2);
    Line(Vec2 pos, float len, float angle);

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
template <typename P>
constexpr Point<P>::Point(const float x, const float y)
    : x{ x }
    , y{ y }
{
}

template <typename P>
constexpr Point<P>::Point(const rl::Vector2 vec)
    : x{ vec.x }
    , y{ vec.y }
{
}

template <typename P>
Point<P>::operator rl::Vector2() const
{
    return { x, y };
}

template <typename P>
auto Point<P>::operator=(const rl::Vector2 vec) -> Point<P>&
{
    x = vec.x;
    y = vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator+(const rl::Vector2 vec) const -> Point<P>
{
    return { x + vec.x, y + vec.y };
}

template <typename P>
auto Point<P>::operator-(const rl::Vector2 vec) const -> Point<P>
{
    return { x - vec.x, y - vec.y };
}

template <typename P>
auto Point<P>::operator*(const float f) const -> Point<P>
{
    return { x * f, y * f };
}

template <typename P>
auto Point<P>::operator/(const float f) const -> Point<P>
{
#ifndef NDEBUG
    if (f == 0.0)
    {
        slog::log(slog::WRN, "Attempted to divide by zero!");

        return { x, y };
    }
#endif

    return { x / f, x / f };
}

template <typename P>
auto Point<P>::operator+=(const rl::Vector2 vec) -> Point<P>&
{
    x += vec.x;
    y += vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator-=(const rl::Vector2 vec) -> Point<P>&
{
    x -= vec.x;
    y -= vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator*=(const float f) -> Point<P>&
{
    x *= f;
    y *= f;

    return *this;
}

template <typename P>
auto Point<P>::operator/=(const float f) -> Point<P>&
{
#ifndef NDEBUG
    if (f == 0.0)
    {
        slog::log(slog::WRN, "Attempted to divide by zero!");

        return *this;
    }
#endif

    x /= f;
    y /= f;

    return *this;
}

template <typename P>
auto Point<P>::len() const -> float
{
    return std::sqrtf((x * x) + (y * y));
}

template <typename P>
auto Point<P>::sqr_len() const -> float
{
    return (x * x) + (y * y);
}

template <typename P>
auto Point<P>::operator-() const -> Point<P>
{
    return { -x, -y };
}

template <typename P>
Point<P>::operator Vec2() const
{
    return { x, y };
}

template <typename P>
auto Point<P>::operator+(Point vec) const -> Point
{
    return { x + vec.x, y + vec.y };
}

template <typename P>
auto Point<P>::operator-(Point vec) const -> Point
{
    return { x - vec.x, y - vec.y };
}

template <typename P>
auto Point<P>::operator*(Point vec) const -> Point
{
    return { x * vec.x, y * vec.y };
}

template <typename P>
auto Point<P>::operator/(Point vec) const -> Point
{
#ifndef NDEBUG
    if (vec.x == 0.0 || vec.y == 0.0)
    {
        slog::log(slog::WRN, "Attempted to divide by zero!");

        return *this;
    }
#endif
}

template <typename P>
auto Point<P>::operator+=(Point vec) -> Point&
{
    x += vec.x;
    y += vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator-=(Point vec) -> Point&
{
    x -= vec.x;
    y -= vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator*=(Point vec) -> Point&
{
    x *= vec.x;
    y *= vec.y;

    return *this;
}

template <typename P>
auto Point<P>::operator/=(Point vec) -> Point&
{
#ifndef NDEBUG
    if (vec.x == 0.0 || vec.y == 0.0)
    {
        slog::log(slog::WRN, "Attempted to divide by zero!");

        return *this;
    }
#endif

    x /= vec.x;
    y /= vec.y;

    return *this;
}

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
