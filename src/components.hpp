#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

#include <optional>
#include <variant>

constexpr float TILE_SIZE = 32.0;

struct Tform {
    RVector2 pos;
    RVector2 vel;

    void move(float dt);

private:
    Tform() = default;

    friend class ComponentManager;
};

struct Sprite {
    RRectangle sprite{ RVector2(0.0, 0.0), RVector2(TILE_SIZE, TILE_SIZE) };

    [[nodiscard]] RVector2 size() const;
    void set_pos(RVector2 pos);
    void flip();
    void unflip();

private:
    Sprite() = default;

    friend class ComponentManager;
};

struct Circle {
    RVector2 pos;
    float radius = 0.0;

    [[nodiscard]] bool check_collision(Circle other_circle) const;
    void draw_lines(::Color color);

private:
    Circle() = default;
    Circle(RVector2 pos, float radius);

    friend class BBox;
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

#endif
