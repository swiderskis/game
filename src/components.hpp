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

enum class SpriteType
{
    PlayerIdle,
    PlayerWalk,
    PlayerJump,
    PlayerFall,
    Projectile,
    Enemy,

    // Tiles
    TileBrick
};

class Sprite
{
    SpriteType m_type = SpriteType::PlayerIdle; // initialise to 0, should always be overwritten for spawned entity
    float m_frame_update_dt = 0.0;
    unsigned m_current_frame = 0;

public:
    bool flipped = false;

    void set(SpriteType type);
    void check_update_frame(float dt);
    [[nodiscard]] RRectangle sprite() const;
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

namespace components
{
std::optional<SpriteType> lookup_movement_sprite(Entity entity, RVector2 vel);
} // namespace components

#endif
