#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "entities.hpp"
#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "settings.hpp"

#include <cstddef>
#include <optional>
#include <variant>

constexpr float TILE_SIZE = 32.0;

struct Tform {
    RVector2 pos;
    RVector2 vel;

private:
    Tform() = default;

    friend struct ComponentManager;
};

enum class SpriteType {
    PlayerIdle,
    PlayerWalk,
    PlayerJump,
    PlayerFall,
    Projectile,
    Enemy,

    // Tiles
    TileBrick
};

struct Sprite {
    SpriteType type = SpriteType::PlayerIdle; // initialise to 0, should be overwritten for every spawned entity anyway
    float frame_update_dt = 0.0;
    unsigned current_frame = 0;
    bool flipped = false;

    void set(SpriteType type);
    void check_update_frame(float dt);
    [[nodiscard]] RRectangle sprite() const;

private:
    // NOLINTBEGIN(*avoid-c-arrays)
    static constexpr struct {
        float x;
        float y;
        float size;
        unsigned frames;
        float frame_duration;
    } DETAILS[] = {
        [(size_t)SpriteType::PlayerIdle] = { 128.0, 0.0, TILE_SIZE, 2, 0.5 },
        [(size_t)SpriteType::PlayerWalk] = { 128.0, 32.0, TILE_SIZE, 4, 0.16 },
        [(size_t)SpriteType::PlayerJump] = { 128.0, 64.0, TILE_SIZE, 1, 0.0 },
        [(size_t)SpriteType::PlayerFall] = { 128.0, 96.0, TILE_SIZE, 4, 0.1 },
        [(size_t)SpriteType::Projectile] = { 128.0, 128.0, TILE_SIZE, 1, 0.0 },
        [(size_t)SpriteType::Enemy] = { 128.0, 160.0, TILE_SIZE, 1, 0.0 },

        // Tiles
        [(size_t)SpriteType::TileBrick] = { 0.0, 0.0, TILE_SIZE, 1, 0.0 },
    };

    // NOLINTEND(*avoid-c-arrays)

    Sprite() = default;

    friend struct ComponentManager;
};

struct Circle {
    RVector2 pos;
    float radius = 0.0;

    Circle(RVector2 pos, float radius);

    [[nodiscard]] bool check_collision(Circle other_circle) const;
    void draw_lines(::Color color) const;

private:
    Circle() = default;

    friend struct ComponentManager;
};

struct BBox {
    std::variant<RRectangle, Circle> bounding_box = RRectangle{ RVector2(0.0, 0.0), RVector2(TILE_SIZE, TILE_SIZE) };

    void sync(Tform transform);
    [[nodiscard]] bool collides(BBox other_bounding_box) const;
    [[nodiscard]] bool x_overlaps(BBox other_bounding_box) const;
    [[nodiscard]] bool y_overlaps(BBox other_bounding_box) const;
    void set_size(RVector2 size);
    void set_size(float radius);

private:
    BBox() = default;
    BBox(RVector2 pos, float radius);

    friend struct ComponentManager;
};

struct Grounded {
    bool grounded = false;

private:
    Grounded() = default;

    friend struct ComponentManager;
};

struct Lifespan {
    std::optional<float> current = std::nullopt;

private:
    Lifespan() = default;

    friend struct ComponentManager;
};

struct Health {
    int current = 0;
    std::optional<int> max = std::nullopt;

    void set_health(int health);
    [[nodiscard]] float percentage() const;

private:
    Health() = default;

    friend struct ComponentManager;
};

struct ComponentManager {
    std::vector<Tform> transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> bounding_boxes{ MAX_ENTITIES, BBox() };
    std::vector<Grounded> grounded{ MAX_ENTITIES, Grounded() };
    std::vector<Lifespan> lifespans{ MAX_ENTITIES, Lifespan() };
    std::vector<Health> health{ MAX_ENTITIES, Health() };

private:
    ComponentManager() = default;

    friend class Game;
};

#endif
