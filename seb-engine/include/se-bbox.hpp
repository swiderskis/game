#ifndef SE_BBOX_HPP_
#define SE_BBOX_HPP_

#include "sl-math.hpp"

#include <cstdint>
#include <variant>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;

struct BBoxRect
{
    sm::Vec2 size;

    BBoxRect() = default;
    explicit constexpr BBoxRect(sm::Vec2 size);
    constexpr BBoxRect(float width, float height);
};

struct BBoxCircle
{
    float radius{ 0 };

    explicit constexpr BBoxCircle(float radius);
};

struct BBoxLine
{
    float len{ 0 };
    float angle{ 0 };

    constexpr BBoxLine(float len, float angle);
};

using BBoxDetails = std::variant<BBoxRect, BBoxCircle, BBoxLine>;
using BBoxVariant = std::variant<rl::Rectangle, sm::Circle, sm::Line>;

class BBox
{
public:
    BBox() = default;
    explicit BBox(BBoxDetails bbox);
    BBox(BBoxDetails bbox, sm::Vec2 offset);

    [[nodiscard]] auto val(sm::Vec2 pos) const -> BBoxVariant;
    [[nodiscard]] auto details() const -> BBoxDetails;

    enum Variant : uint8_t
    {
        RECTANGLE,
        CIRCLE,
        LINE,
    };

private:
    BBoxDetails m_bbox;
    sm::Vec2 m_offset;
};

namespace bbox
{
auto collides(BBoxVariant bbox1, BBoxVariant bbox2) -> bool;
auto resolve_collision(BBoxVariant bbox1, BBoxVariant bbox2) -> sm::Vec2;
} // namespace bbox
} // namespace seb_engine

/****************************
 *                          *
 * TEMPLATE IMPLEMENTATIONS *
 *                          *
 ****************************/

namespace seb_engine
{
constexpr BBoxRect::BBoxRect(const sm::Vec2 size)
    : size{ size }
{
}

constexpr BBoxRect::BBoxRect(const float width, const float height)
    : size{ width, height }
{
}

constexpr BBoxCircle::BBoxCircle(const float radius)
    : radius{ radius }
{
}

constexpr BBoxLine::BBoxLine(const float len, const float angle)
    : len{ len }
    , angle{ angle }
{
}
} // namespace seb_engine

#endif
