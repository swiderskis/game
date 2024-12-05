#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

// Taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Ts>
struct overloaded : Ts... // NOLINT(readability-identifier-naming)
{
    using Ts::operator()...;
};

struct SimpleVec2 // exists to allow constexpr vec declarations
{
    float x;
    float y;

    SimpleVec2() = delete;

    constexpr SimpleVec2(float x, float y) : x(x), y(y)
    {
    }

    operator RVector2() const // NOLINT(hicpp-explicit-conversions)
    {
        return { x, y };
    }
};

#endif