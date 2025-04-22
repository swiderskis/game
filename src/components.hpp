#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "settings.hpp"
#include "utils.hpp"

#include <bitset>
#include <cstdint>
#include <optional>
#include <variant>

inline constexpr float TILE_SIZE = 16.0;
inline constexpr float SPRITE_SIZE = 32.0;

inline constexpr unsigned FLAG_COUNT = 8;

struct Tform
{
    RVector2 pos;
    RVector2 vel;
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
    SimpleVec2 pos;
    SimpleVec2 size;
    unsigned frames;
    float frame_duration;
    bool allow_movement_override;
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

struct Circle
{
    RVector2 pos;
    float radius = 0.0;

    Circle(RVector2 pos, float radius);

    void draw_lines(::Color color) const;
};

struct Line
{
    RVector2 pos1;
    RVector2 pos2;

    Line() = delete;
    Line(RVector2 pos1, RVector2 pos2);
    Line(RVector2 pos, float len, float angle);

    [[nodiscard]] float len() const;
    void draw_line(::Color color) const;
};

using BBoxVariant = std::variant<RRectangle, Circle, Line>;

class BBox
{
    BBoxVariant m_bounding_box{ RRectangle() };

public:
    RVector2 offset{ 0.0, 0.0 };

    BBox() = default;

    void sync(Tform transform, bool flipped);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;
    void set(Tform transform, RVector2 size);
    void set(Tform transform, float radius);
    void set(Tform transform, float len, float angle);
    [[nodiscard]] BBoxVariant bounding_box() const;

    enum Variant : uint8_t
    {
        RECTANGLE = 0,
        CIRCLE = 1,
        LINE = 2,
    };
};

namespace flag
{
enum Flag : uint8_t
{
    FLIPPED,
};
} // namespace flag

struct Health
{
    int current = 0;
    std::optional<int> max = std::nullopt;

    void set_health(int health);
    [[nodiscard]] float percentage() const;
};

class EntityComponents;

struct Components
{
    std::vector<Tform> transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> collision_boxes{ MAX_ENTITIES, BBox() };
    std::vector<std::bitset<FLAG_COUNT>> flags;
    std::vector<std::optional<float>> lifespans;
    std::vector<Health> healths{ MAX_ENTITIES, Health() };
    std::vector<BBox> hitboxes{ MAX_ENTITIES, BBox() };
    std::vector<std::optional<unsigned>> parents;
    std::vector<float> attack_cooldowns;
    std::vector<float> invuln_times;
    std::vector<unsigned> hit_damage;

    Components();

    void uninit_destroyed_entity(unsigned id);
    [[nodiscard]] EntityComponents get_by_id(unsigned id);
};

class EntityComponents
{
    Components* m_components;
    unsigned m_id;

    EntityComponents(Components& components, unsigned id);
    friend struct Components;

public:
    EntityComponents() = delete;

    EntityComponents& set_pos(RVector2 pos);
    EntityComponents& set_vel(RVector2 vel);
    EntityComponents& set_cbox_size(RVector2 cbox_size);
    EntityComponents& set_cbox_size(float radius);
    EntityComponents& set_cbox_size(float len, float angle);
    EntityComponents& set_cbox_offset(RVector2 offset);
    EntityComponents& set_health(int health);
    EntityComponents& set_hitbox_size(RVector2 hbox_size);
    EntityComponents& set_hitbox_size(float radius);
    EntityComponents& set_hitbox_size(float len, float angle);
    EntityComponents& set_hitbox_offset(RVector2 offset);
    EntityComponents& set_sprite_base(SpriteBase sprite);
    EntityComponents& set_sprite_head(SpriteHead sprite);
    EntityComponents& set_sprite_arms(SpriteArms sprite);
    EntityComponents& set_sprite_legs(SpriteLegs sprite);
    EntityComponents& set_sprite_extra(SpriteExtra sprite);
    EntityComponents& set_lifespan(float lifespan);
    EntityComponents& set_parent(unsigned parent);
    EntityComponents& set_hit_damage(unsigned damage);
};

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
