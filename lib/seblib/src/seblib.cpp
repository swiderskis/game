#include "seblib.hpp"

namespace seblib
{
SimpleVec2::operator raylib::Vector2() const
{
    return { x, y };
}
} // namespace seblib
