#include "components.hpp"

#include <cassert>
#include <cmath>
#include <optional>

bool Flags::is_enabled(const Flag flag_enum) const
{
    return flag[flag_enum];
}

void Flags::set(const Flag flag_enum, const bool val)
{
    flag[flag_enum] = val;
}

void Health::set(const int health)
{
    current = health;
    max = health;
}

float Health::percentage() const
{
    assert(max != std::nullopt);

    return static_cast<float>(current) / static_cast<float>(max.value());
}
