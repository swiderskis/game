#ifndef SEB_ENGINE_HPP_
#define SEB_ENGINE_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-math.hpp"

#include <cstddef>
#include <format>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;

template <unsigned CoordSize>
struct Coords
{
    size_t x{ 0 };
    size_t y{ 0 };

    constexpr Coords(size_t x, size_t y);

    static auto from_vec2(sm::Vec2 pos) -> std::optional<Coords<CoordSize>>;

    operator rl::Vector2() const;        // NOLINT(hicpp-explicit-conversions)
    constexpr operator sm::Vec2() const; // NOLINT(hicpp-explicit-conversions)
    [[nodiscard]] auto operator+(Coords coords) const -> Coords;
    [[nodiscard]] auto operator-(Coords coords) const -> Coords;
    [[maybe_unused]] auto operator+=(Coords coords) -> Coords&;
    [[maybe_unused]] auto operator-=(Coords coords) -> Coords&;
};
} // namespace seb_engine

template <unsigned CoordSize>
struct std::formatter<seb_engine::Coords<CoordSize>> // NOLINT(cert-dcl58-cpp)
{
    std::formatter<std::string> formatter;

    constexpr auto parse(std::format_parse_context& ctx);
    auto format(seb_engine::Coords<CoordSize> const& coords, std::format_context& ctx) const;
};

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
template <unsigned CoordSize>
constexpr Coords<CoordSize>::Coords(const size_t x, const size_t y)
    : x{ x }
    , y{ y }
{
}

template <unsigned CoordSize>
auto Coords<CoordSize>::from_vec2(const sm::Vec2 pos) -> std::optional<Coords<CoordSize>>
{
    return Coords<CoordSize>{ (static_cast<unsigned>(pos.x + CoordSize) / CoordSize),
                              static_cast<unsigned>(-pos.y) / CoordSize };
}

template <unsigned CoordSize>
Coords<CoordSize>::operator rl::Vector2() const
{
    return { static_cast<float>(x) * CoordSize, -static_cast<float>(y) * CoordSize };
}

template <unsigned CoordSize>
constexpr Coords<CoordSize>::operator sm::Vec2() const
{
    return { static_cast<float>(x) * CoordSize, -static_cast<float>(y) * CoordSize };
}

template <unsigned CoordSize>
auto Coords<CoordSize>::operator+(const Coords coords) const -> Coords
{
    return { x + coords.x, y + coords.y };
}

template <unsigned CoordSize>
auto Coords<CoordSize>::operator-(const Coords coords) const -> Coords
{
    return { x - coords.x, y - coords.y };
}

template <unsigned CoordSize>
auto Coords<CoordSize>::operator+=(const Coords coords) -> Coords&
{
    x += coords.x;
    y += coords.y;

    return *this;
}

template <unsigned CoordSize>
auto Coords<CoordSize>::operator-=(const Coords coords) -> Coords&
{
    x -= coords.x;
    y -= coords.y;

    return *this;
}
} // namespace seb_engine

template <unsigned CoordSize>
constexpr auto std::formatter<seb_engine::Coords<CoordSize>>::parse(std::format_parse_context& ctx)
{
    return formatter.parse(ctx);
}

template <unsigned CoordSize>
auto std::formatter<seb_engine::Coords<CoordSize>>::format(
    seb_engine::Coords<CoordSize> const& coords, std::format_context& ctx
) const
{
    const std::string output{ std::format("({}, {})", coords.x, coords.y) };

    return formatter.format(output, ctx);
}
#endif
