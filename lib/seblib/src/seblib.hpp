#ifndef SEBLIB_HPP_
#define SEBLIB_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <variant>

namespace seblib
{
namespace rl = raylib;

// exists to allow constexpr vec declarations
struct SimpleVec2
{
    float x;
    float y;

    SimpleVec2() = delete;

    constexpr SimpleVec2(float x, float y);

    operator rl::Vector2() const; // NOLINT(hicpp-explicit-conversions)
};

// taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Funcs>
struct Overload : Funcs...
{
    using Funcs::operator()...;
};

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
constexpr SimpleVec2::SimpleVec2(const float x, const float y) : x(x), y(y)
{
}

template <typename Var, typename... Funcs>
auto match(Var&& variant, Funcs&&... funcs)
{
    return std::visit(Overload{ std::forward<Funcs>(funcs)... }, std::forward<Var>(variant));
}
} // namespace seblib

#endif
