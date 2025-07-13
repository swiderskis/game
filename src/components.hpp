#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seblib-math.hpp"

#include <bitset>
#include <cstdint>
#include <optional>
#include <variant>

inline constexpr float TILE_SIZE = 16.0;

inline constexpr unsigned FLAG_COUNT = 8;

using BBoxVariant = std::variant<raylib::Rectangle, seblib::math::Circle, seblib::math::Line>;

class BBox
{
    BBoxVariant m_bbox{ raylib::Rectangle() };
    raylib::Vector2 m_offset{ 0.0, 0.0 };

public:
    BBox() = default;

    void sync(raylib::Vector2 pos, bool flipped);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;
    void set(raylib::Vector2 pos, raylib::Vector2 size);
    void set(raylib::Vector2 pos, float radius);
    void set(raylib::Vector2 pos, float len, float angle);
    void set_offset(raylib::Vector2 pos, raylib::Vector2 offset);
    [[nodiscard]] BBoxVariant bbox() const;

    enum Variant : uint8_t
    {
        RECTANGLE = 0,
        CIRCLE = 1,
        LINE = 2,
    };
};

struct Tform
{
    raylib::Vector2 pos;
    raylib::Vector2 vel;
    BBox cbox;

    Tform() = default;
};

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
    BBox hitbox;
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
