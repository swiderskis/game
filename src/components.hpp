#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

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

    friend class ComponentManager;
};

enum class SpriteType {
    Player,
    TileBrick,
    Projectile,
    Enemy
};

struct Sprite {
    RRectangle sprite{ RVector2(0.0, 0.0), RVector2(TILE_SIZE, TILE_SIZE) };

    void set_sprite(SpriteType type);
    void flip();
    void unflip();

private:
    // NOLINTBEGIN(*avoid-c-arrays)
    constexpr static struct {
        float x;
        float y;
    } SHEET_POS[] = {
        [(size_t)SpriteType::Player] = { 0.0, 0.0 },
        [(size_t)SpriteType::TileBrick] = { 0.0, 32.0 },
        [(size_t)SpriteType::Projectile] = { 0.0, 64.0 },
        [(size_t)SpriteType::Enemy] = { 0.0, 96.0 },
    };

    // NOLINTEND(*avoid-c-arrays)

    Sprite() = default;

    friend class ComponentManager;
};

struct Circle {
    RVector2 pos;
    float radius = 0.0;

    Circle(RVector2 pos, float radius);

    [[nodiscard]] bool check_collision(Circle other_circle) const;
    void draw_lines(::Color color) const;

private:
    Circle() = default;

    friend class ComponentManager;
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

    friend class ComponentManager;
};

struct Grounded {
    bool grounded = false;

private:
    Grounded() = default;

    friend class ComponentManager;
};

struct Lifespan {
    std::optional<float> current = std::nullopt;

private:
    Lifespan() = default;

    friend class ComponentManager;
};

struct Health {
    int current = 0;
    std::optional<int> max = std::nullopt;

    void set_health(int health);
    [[nodiscard]] float percentage() const;

private:
    Health() = default;

    friend class ComponentManager;
};

class ComponentManager
{
    std::vector<Tform> m_transforms{ MAX_ENTITIES, Tform() };
    std::vector<Sprite> m_sprites{ MAX_ENTITIES, Sprite() };
    std::vector<BBox> m_bounding_boxes{ MAX_ENTITIES, BBox() };
    std::vector<Grounded> m_grounded{ MAX_ENTITIES, Grounded() };
    std::vector<Lifespan> m_lifespans{ MAX_ENTITIES, Lifespan() };
    std::vector<Health> m_health{ MAX_ENTITIES, Health() };

    ComponentManager() = default;

    friend class Game;
};

#endif
