#include "utils.hpp"

#include <cmath>

float utils::degrees_to_radians(float ang)
{
    return (float)(ang * M_PI / 180.0); // NOLINT(*magic-numbers)
}

float utils::radians_to_degrees(float ang)
{
    return (float)(ang * 180.0 / M_PI); // NOLINT(*magic-numbers)
}
