#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "se-ecs.hpp"

#include <bitset>
#include <cstdint>
#include <optional>

inline constexpr size_t FLAG_COUNT{ 8 };

struct Flags
{
    std::bitset<FLAG_COUNT> flag;

    enum Flag : uint8_t
    {
        FLIPPED,
    };

    [[nodiscard]] auto is_enabled(Flag flag) const -> bool;
    auto set(Flag flag, bool val) -> void;
};

struct Health
{
    std::optional<int> max;
    int current{ 0 };

    auto set(int health) -> void;
    [[nodiscard]] auto percentage() const -> float;
};

struct Combat
{
    seb_engine::BBox hitbox;
    Health health;
    std::optional<float> lifespan;
    float attack_cooldown{ 0.0 };
    float invuln_time{ 0.0 };
    unsigned damage{ 0 };
};

struct Parent
{
    std::optional<unsigned> id;
};

#endif
