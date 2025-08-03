#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seb-engine-ecs.hpp"

#include <bitset>
#include <cstdint>
#include <optional>

inline constexpr unsigned FLAG_COUNT = 8;

struct Flags
{
    std::bitset<FLAG_COUNT> flag;

    enum Flag : uint8_t
    {
        FLIPPED,
    };

    [[nodiscard]] bool is_enabled(Flag flag) const;
    void set(Flag flag, bool val);
};

struct Health

{
    std::optional<int> max = std::nullopt;
    int current = 0;

    void set(int health);
    [[nodiscard]] float percentage() const;
};

struct Combat
{
    seb_engine::BBox hitbox;
    Health health;
    std::optional<float> lifespan = std::nullopt;
    float attack_cooldown = 0.0;
    float invuln_time = 0.0;
    unsigned damage = 0;
};

struct Parent
{
    std::optional<unsigned> id;
};

#endif
