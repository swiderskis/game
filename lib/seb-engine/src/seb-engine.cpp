#include "seb-engine.hpp"

namespace seb_engine
{
Coord::operator rl::Vector2() const
{
    return pos;
}
} // namespace seb_engine
