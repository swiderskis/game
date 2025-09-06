#include "components.hpp"

#include <cassert>
#include <cmath>
#include <optional>

auto Flags::is_enabled(const Flag flag_enum) const -> bool
{
    return flag[flag_enum];
}

auto Flags::set(const Flag flag_enum, const bool val) -> void
{
    flag[flag_enum] = val;
}

auto Health::set(const int health) -> void
{
    current = health;
    max = health;
}

auto Health::percentage() const -> float
{
    assert(max != std::nullopt);

    return static_cast<float>(current) / static_cast<float>(max.value());
}
