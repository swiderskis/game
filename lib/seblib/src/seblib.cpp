#include "seblib.hpp"

namespace sl = seblib;

sl::SimpleVec2::operator raylib::Vector2() const
{
    return { x, y };
}
