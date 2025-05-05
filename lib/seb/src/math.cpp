#include "seb.hpp"

#include <cmath>

namespace seb::math
{
float degrees_to_radians(float ang)
{
    return (float)(ang * M_PI / 180.0); // NOLINT(*magic-numbers)
}

float radians_to_degrees(float ang)
{
    return (float)(ang * 180.0 / M_PI); // NOLINT(*magic-numbers)
}
} // namespace seb::math
