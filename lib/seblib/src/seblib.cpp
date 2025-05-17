#include "seblib.hpp"

namespace seblib
{
namespace rl = raylib;

SimpleVec2::operator rl::Vector2() const
{
    return { x, y };
}
} // namespace seblib
