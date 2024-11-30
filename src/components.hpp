#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "settings.hpp"

#include <optional>
#include <variant>

inline constexpr float TILE_SIZE = 32.0;

struct Tform
{
    RVector2 pos;
    RVector2 vel;
};

enum class SpriteBase
{
    None = -1,

    PlayerIdle,
    Projectile,
    Enemy,

    // tiles
    TileBrick,
};

enum class SpriteHead
{
    None = -1,

    PlayerIdle,
};

enum class SpriteArms
{
    None = -1,

    PlayerIdle,
    PlayerJump,
};

enum class SpriteLegs
{
    None = -1,

    PlayerIdle,
    PlayerWalk,
    PlayerJump,
};

enum class SpriteExtra
{
    None = -1,

    PlayerScarfWalk,
    PlayerScarfFall,
};

struct SpriteDetails
{
    float x;
    float y;
    float size;
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

template <typename S>
class SpritePart
{
    S m_type;
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;

public:
    SpritePart() = delete;

    explicit SpritePart(S type) : m_type(type)
    {
    }

    void set(S type)
    {
        if (m_type == type)
        {
            return;
        }

        m_type = type;
        m_current_frame = 0;
        m_frame_update_dt = 0.0;
    }

    void check_update_frame(float dt)
    {
        const auto details = components::sprite_details(m_type);
        if (details.frames == 1)
        {
            return;
        }

        m_frame_update_dt += dt;
        if (m_frame_update_dt < details.frame_duration)
        {
            return;
        }

        m_frame_update_dt = 0.0;
        m_current_frame += 1;
        if (m_current_frame == details.frames)
        {
            m_current_frame = 0;
        }
    }

    [[nodiscard]] RRectangle sprite(bool flipped) const
    {
        const auto details = components::sprite_details(m_type);
        const auto pos = RVector2(details.x + (details.size * (float)m_current_frame), details.y);
        const auto size = RVector2(details.size * (flipped ? -1.0 : 1.0), details.size);

        return { pos, size };
    }

    [[nodiscard]] S type() const
    {
        return m_type;
    }

    [[nodiscard]] unsigned current_frame()
    {
        return m_current_frame;
    }

    void movement_set(S type)
    {
        if (components::sprite_details(m_type).allow_movement_override)
        {
            set(type);
        }
    }
};

struct Sprite
{
    SpritePart<SpriteBase> base{ SpriteBase::None };
    SpritePart<SpriteHead> head{ SpriteHead::None };
    SpritePart<SpriteArms> arms{ SpriteArms::None };
    SpritePart<SpriteLegs> legs{ SpriteLegs::None };
    SpritePart<SpriteExtra> extra{ SpriteExtra::None };
    bool flipped = false;

    void check_update_frames(float dt);
    void draw(RTexture const& texture_sheet, Tform transform);
    void lookup_set_movement_parts(Entity entity, RVector2 vel);

private:
    [[nodiscard]] float alternate_frame_y_offset() const;
    void lookup_set_fall_parts(Entity entity);
    void lookup_set_jump_parts(Entity entity);
    void lookup_set_walk_parts(Entity entity);
    void lookup_set_idle_parts(Entity entity);
};

struct Circle
{
    RVector2 pos;
    float radius = 0.0;

    Circle(RVector2 pos, float radius);

    [[nodiscard]] bool check_collision(Circle other_circle) const;
    void draw_lines(::Color color) const;
};

class BBox
{
    std::variant<RRectangle, Circle> m_bounding_box = RRectangle{ RVector2(0.0, 0.0), RVector2(TILE_SIZE, TILE_SIZE) };

public:
    BBox() = default;

    void sync(Tform transform);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;
    void set(Tform transform, RVector2 size);
    void set(Tform transform, float radius);
    [[nodiscard]] std::variant<RRectangle, Circle> bounding_box() const;
};

struct Grounded
{
    bool grounded = false;
};

struct Lifespan
{
    std::optional<float> current = std::nullopt;
};

struct Health
{
    int current = 0;
    std::optional<int> max = std::nullopt;

    void set_health(int health);
    [[nodiscard]] float percentage() const;
};

struct Components
{
    std::vector<Tform> transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> bounding_boxes{ MAX_ENTITIES, BBox() };
    std::vector<Grounded> grounded{ MAX_ENTITIES, Grounded() };
    std::vector<Lifespan> lifespans{ MAX_ENTITIES, Lifespan() };
    std::vector<Health> health{ MAX_ENTITIES, Health() };
};

#endif
