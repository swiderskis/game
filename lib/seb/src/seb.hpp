#ifndef SEB_HPP_
#define SEB_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

namespace seb
{
// exists to allow constexpr vec declarations
struct SimpleVec2
{
    float x;
    float y;

    SimpleVec2() = delete;

    constexpr SimpleVec2(float x, float y) : x(x), y(y)
    {
    }

    constexpr operator RVector2() const // NOLINT(hicpp-explicit-conversions)
    {
        return { x, y };
    }
};

namespace math
{
float degrees_to_radians(float ang);
float radians_to_degrees(float ang);
} // namespace math
} // namespace seb

#endif
