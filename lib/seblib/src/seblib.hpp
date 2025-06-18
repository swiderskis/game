#ifndef SEBLIB_HPP_
#define SEBLIB_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <cstdlib>
#include <iostream>
#include <numbers>
#include <source_location>
#include <variant>

inline constexpr unsigned FILENAME_WIDTH = 8;

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

//
// taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <typename... Funcs>
struct Overload : Funcs...
{
    using Funcs::operator()...;
};

// taken from https://www.reddit.com/r/cpp/comments/16lq63k/2_lines_of_code_and_3_c17_features_the_overload
template <typename Var, typename... Funcs>
auto match(Var&& variant, Funcs&&... funcs);

namespace math
{
struct Circle
{
    RVector2 pos;
    float radius = 0.0;

    Circle(RVector2 pos, float radius);

    void draw_lines(rl::Color color) const;
};

struct Line
{
    RVector2 pos1;
    RVector2 pos2;

    Line() = delete;
    Line(RVector2 pos1, RVector2 pos2);
    Line(RVector2 pos, float len, float angle);

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
} // namespace math

namespace log
{
enum Level : int8_t
{
    FTL = -1,
    ERR = 0,
    WRN = 1,
    INF = 2,
    TRC = 3,
};

// taken from https://stackoverflow.com/questions/14805192/c-variadic-template-function-parameter-with-default-value
template <typename... Args>
struct log // NOLINT(readability-identifier-naming)
{
    explicit log(Level lvl,
                 std::format_string<Args...> fmt,
                 Args&&... args,
                 std::source_location loc = std::source_location::current());
};

template <typename... Args>
log(Level level, std::format_string<Args...> fmt, Args&&... args) -> log<Args...>;
int level();
void set_level(int level);
} // namespace log
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

namespace math
{
constexpr float degrees_to_radians(const float ang)
{
    return static_cast<float>(ang * std::numbers::pi / 180.0); // NOLINT(*magic-numbers)
}

constexpr float radians_to_degrees(const float ang)
{
    return static_cast<float>(ang * 180.0 / std::numbers::pi); // NOLINT(*magic-numbers)
}
} // namespace math

namespace log
{
template <typename... Args>
log<Args...>::log(const Level lvl,
                  const std::format_string<Args...> fmt,
                  Args&&... args,
                  const std::source_location loc)
{
    if (level() > lvl)
    {
        std::string level_text;
        switch (lvl)
        {
        case FTL:
            level_text = "FTL";
            break;
        case ERR:
            level_text = "ERR";
            break;
        case WRN:
            level_text = "WRN";
            break;
        case INF:
            level_text = "INF";
            break;
        case TRC:
            level_text = "TRC";
            break;
        }

        const auto filename = std::string(loc.file_name());
        std::clog << "[" << level_text << "] " << filename.substr(filename.size() - FILENAME_WIDTH)
                  << std::format(" {:>5}: ", loc.line()) << std::format(fmt, std::forward<Args>(args)...) << "\n";

        if (lvl == FTL)
        {
            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        }
    }
}
} // namespace log
} // namespace seblib

#endif
