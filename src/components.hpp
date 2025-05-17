#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "seblib.hpp"

#include <bitset>
#include <cstdint>
#include <optional>
#include <variant>

inline constexpr float TILE_SIZE = 16.0;
inline constexpr float SPRITE_SIZE = 32.0;

inline constexpr unsigned FLAG_COUNT = 8;

using BBoxVariant = std::variant<RRectangle, seblib::math::Circle, seblib::math::Line>;

class BBox
{
    BBoxVariant m_bbox{ RRectangle() };
    RVector2 m_offset{ 0.0, 0.0 };

public:
    BBox() = default;

    void sync(RVector2 pos, bool flipped);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;
    void set(RVector2 pos, RVector2 size);
    void set(RVector2 pos, float radius);
    void set(RVector2 pos, float len, float angle);
    void set_offset(RVector2 pos, RVector2 offset);
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
    RVector2 pos;
    RVector2 vel;
    BBox cbox;

    Tform() = default;
};

enum class SpriteBase : int8_t
{
    None = -1,

    PlayerIdle,
    Projectile,
    EnemyDuck,

    // tiles
    TileBrick,
};

enum class SpriteHead : int8_t
{
    None = -1,

    PlayerIdle,
};

enum class SpriteArms : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerJump,
    PlayerAttack,
};

enum class SpriteLegs : int8_t
{
    None = -1,

    PlayerIdle,
    PlayerWalk,
    PlayerJump,
};

enum class SpriteExtra : int8_t
{
    None = -1,

    PlayerScarfWalk,
    PlayerScarfFall,
};

struct SpriteDetails
{
    RVector2 pos;
    RVector2 size;
    unsigned frames = 0;
    float frame_duration = 0.0;
    bool allow_movement_override = false;
};

namespace components
{
SpriteDetails sprite_details(SpriteBase sprite);
SpriteDetails sprite_details(SpriteHead sprite);
SpriteDetails sprite_details(SpriteArms sprite);
SpriteDetails sprite_details(SpriteLegs sprite);
SpriteDetails sprite_details(SpriteExtra sprite);
} // namespace components

template <typename Part>
class SpritePart
{
    Part m_part;
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;
    float m_frame_duration = 0.0;

public:
    SpritePart() = delete;
    explicit SpritePart(Part part);

    void set(Part part);
    void set(Part part, float frame_duration);
    void check_update_frame(float dt);
    [[nodiscard]] RRectangle sprite(bool flipped) const;
    [[nodiscard]] Part part() const;
    [[nodiscard]] unsigned current_frame() const;
    void movement_set(Part part);
};

struct Sprite
{
    SpritePart<SpriteBase> base{ SpriteBase::None };
    SpritePart<SpriteHead> head{ SpriteHead::None };
    SpritePart<SpriteArms> arms{ SpriteArms::None };
    SpritePart<SpriteLegs> legs{ SpriteLegs::None };
    SpritePart<SpriteExtra> extra{ SpriteExtra::None };

    void check_update_frames(float dt);
    void draw(RTexture const& texture_sheet, Tform transform, bool flipped);
    void lookup_set_movement_parts(Entity entity, RVector2 vel);

private:
    [[nodiscard]] float alternate_frame_y_offset() const;
    void lookup_set_fall_parts(Entity entity);
    void lookup_set_jump_parts(Entity entity);
    void lookup_set_walk_parts(Entity entity);
    void lookup_set_idle_parts(Entity entity);
    template <typename Part>
    [[nodiscard]] RVector2 render_pos(SpritePart<Part> part, RVector2 pos, bool flipped) const;
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

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

template <typename Part>
SpritePart<Part>::SpritePart(const Part part) : m_part(part)

{
}

template <typename Part>
void SpritePart<Part>::set(const Part part)
{
    if (m_part == part)
    {
        return;
    }

    const auto details = components::sprite_details(part);
    m_part = part;
    m_current_frame = 0;
    m_frame_update_dt = details.frame_duration;
    m_frame_duration = details.frame_duration;
}

template <typename Part>
void SpritePart<Part>::set(const Part part, const float frame_duration)
{
    if (m_part == part)
    {
        return;
    }

    m_part = part;
    m_current_frame = 0;
    m_frame_update_dt = frame_duration;
    m_frame_duration = frame_duration;
}

template <typename Part>
void SpritePart<Part>::check_update_frame(const float dt)
{
    const auto details = components::sprite_details(m_part);
    if (details.frame_duration == 0.0)
    {
        return;
    }

    m_frame_update_dt -= dt;
    if (m_frame_update_dt > 0)
    {
        return;
    }

    m_frame_update_dt = m_frame_duration;
    m_current_frame += 1;
    if (m_current_frame == details.frames)
    {
        set((Part)-1);
    }
}

template <typename Part>
RRectangle SpritePart<Part>::sprite(const bool flipped) const
{
    const auto details = components::sprite_details(m_part);
    const auto pos = RVector2(details.size.x * m_current_frame, 0.0) + details.pos;
    const auto size = RVector2((flipped ? -1.0F : 1.0F), 1.0) * details.size;

    return { pos, size };
}

template <typename Part>
Part SpritePart<Part>::part() const
{
    return m_part;
}

template <typename Part>
unsigned SpritePart<Part>::current_frame() const
{
    return m_current_frame;
}

template <typename Part>
void SpritePart<Part>::movement_set(const Part part)
{
    if (components::sprite_details(m_part).allow_movement_override)
    {
        set(part);
    }
}

template <typename Part>
RVector2 Sprite::render_pos(const SpritePart<Part> part, const RVector2 pos, const bool flipped) const
{
    // sprite part draw pos needs to be offset if it is wider than default sprite size and the sprite is flipped
    const float x_offset = (components::sprite_details(part.part()).size.x - SPRITE_SIZE) * flipped;

    return pos - RVector2(x_offset, 0.0);
}

#endif
