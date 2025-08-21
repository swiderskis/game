#ifndef SEBLIB_HPP_
#define SEBLIB_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <type_traits>
#include <variant>

namespace seblib
{
namespace rl = raylib;

template <typename P>
struct Point
{
    float x = 0.0;
    float y = 0.0;

    Point() = default;
    constexpr Point(float x, float y);

    operator rl::Vector2() const; // NOLINT(hicpp-explicit-conversions)
    Point& operator=(rl::Vector2 vec);
    Point operator+(rl::Vector2 vec) const;
    Point operator-(rl::Vector2 vec) const;
    Point operator*(float f) const;
    Point operator/(float f) const;
    Point& operator+=(rl::Vector2 vec);
    Point& operator-=(rl::Vector2 vec);
    Point& operator*=(float f);
    Point& operator/=(float f);
};

// taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Funcs>
struct Overload : Funcs...
{
    using Funcs::operator()...;
};

template <typename Enum>
concept Enumerable = std::is_enum_v<Enum> || std::is_scoped_enum_v<Enum>;

struct SV2;
using SimpleVec2 = Point<SV2>;

// taken from https://www.reddit.com/r/cpp/comments/16lq63k/2_lines_of_code_and_3_c17_features_the_overload
template <typename Var, typename... Funcs>
auto match(Var&& variant, Funcs&&... funcs);
} // namespace seblib

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seblib
{
template <typename P>
constexpr Point<P>::Point(const float x, const float y) : x(x), y(y)
{
}

template <typename P>
Point<P>::operator rl::Vector2() const
{
    return { x, y };
}

template <typename P>
Point<P>& Point<P>::operator=(const rl::Vector2 vec)
{
    x = vec.x;
    y = vec.y;

    return *this;
}

template <typename P>
Point<P> Point<P>::operator+(const rl::Vector2 vec) const
{
    return Point<P>{ x + vec.x, y + vec.y };
}

template <typename P>
Point<P> Point<P>::operator-(const rl::Vector2 vec) const
{
    return Point<P>{ x - vec.x, y - vec.y };
}

template <typename P>
Point<P> Point<P>::operator*(const float f) const
{
    return Point<P>{ x * f, y * f };
}

template <typename P>
Point<P> Point<P>::operator/(const float f) const
{
    return Point<P>{ x / f, x / f };
}

template <typename P>
Point<P>& Point<P>::operator+=(const rl::Vector2 vec)
{
    x += vec.x;
    y += vec.y;

    return *this;
}

template <typename P>
Point<P>& Point<P>::operator-=(const rl::Vector2 vec)
{
    x -= vec.x;
    y -= vec.y;

    return *this;
}

template <typename P>
Point<P>& Point<P>::operator*=(const float f)
{
    x *= f;
    y *= f;

    return *this;
}

template <typename P>
Point<P>& Point<P>::operator/=(const float f)
{
    x /= f;
    y /= f;

    return *this;
}

template <typename Var, typename... Funcs>
auto match(Var&& variant, Funcs&&... funcs)
{
    return std::visit(Overload{ std::forward<Funcs>(funcs)... }, std::forward<Var>(variant));
}
} // namespace seblib

#endif
