#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif

// taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Ts>
struct overloaded : Ts... // NOLINT(readability-identifier-naming)
{
    using Ts::operator()...;
};

// exists to allow constexpr vec declarations
struct SimpleVec2
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

// this should be made into a library / libraries if it grows enough
namespace utils
{
float degrees_to_radians(float ang);
float radians_to_degrees(float ang);
} // namespace utils

#endif
