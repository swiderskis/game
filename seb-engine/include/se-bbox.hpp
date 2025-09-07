#ifndef SE_BBOX_HPP_
#define SE_BBOX_HPP_

#include "raylib-cpp.hpp" // IWYU pragma: keep
#include "sl-math.hpp"

#include <cstdint>
#include <variant>

namespace seb_engine
{
namespace rl = raylib;
namespace sm = seblib::math;

using BBoxVariant = std::variant<rl::Rectangle, sm::Circle, sm::Line>;

class BBox
{
public:
    BBox() = default;
    explicit BBox(BBoxVariant bbox);
    explicit BBox(BBoxVariant bbox, rl::Vector2 offset);

    auto sync(rl::Vector2 pos) -> void;
    [[nodiscard]] auto collides(BBox other_bbox) const -> bool;
    [[nodiscard]] auto x_overlaps(BBox other_bbox) const -> bool;
    [[nodiscard]] auto y_overlaps(BBox other_bbox) const -> bool;
    [[nodiscard]] auto val() const -> BBoxVariant;

    enum Variant : uint8_t
    {
        RECTANGLE,
        CIRCLE,
        LINE,
    };

private:
    BBoxVariant m_bbox;
    rl::Vector2 m_offset;
};
} // namespace seb_engine

#endif
