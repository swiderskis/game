#include "seb-engine.hpp"

namespace seb_engine
{
Coords::operator rl::Vector2() const
{
    return { static_cast<float>(x) * COORD_SIZE, -static_cast<float>(y) * COORD_SIZE };
}
} // namespace seb_engine
