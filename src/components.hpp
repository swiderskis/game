#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep

constexpr float SPRITE_DEFAULT_SIZE = 32.0;

struct Tform {
    RVector2 pos;
    RVector2 vel;

    void move();
    [[nodiscard]] RVector2 previous_pos() const;

private:
    Tform() = default;

    friend class ComponentManager;
};

struct Sprite {
    RRectangle sprite{ RVector2(0.0, 0.0), RVector2(SPRITE_DEFAULT_SIZE, SPRITE_DEFAULT_SIZE) };

    [[nodiscard]] RVector2 size() const;
    void set_pos(RVector2 pos);

private:
    Sprite() = default;

    friend class ComponentManager;
};

struct BBox {
    RRectangle bounding_box{ RVector2(0.0, 0.0), RVector2(SPRITE_DEFAULT_SIZE, SPRITE_DEFAULT_SIZE) };

    void sync(Tform transform);
    [[nodiscard]] bool collides(BBox other_bbox) const;
    [[nodiscard]] bool x_overlaps(BBox other_bbox) const;
    [[nodiscard]] bool y_overlaps(BBox other_bbox) const;

private:
    BBox() = default;

    friend class ComponentManager;
};

#endif
