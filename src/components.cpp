#include "components.hpp"

#include <cassert>
#include <cmath>
#include <optional>

auto Flags::is_enabled(Flag const flag_enum) const -> bool
{
    return flag[flag_enum];
}

auto Flags::set(Flag const flag_enum, bool const val) -> void
{
    flag[flag_enum] = val;
}

auto Health::set(int const health) -> void
{
    current = health;
    max = health;
}

auto Health::percentage() const -> float
{
    assert(max != std::nullopt);

    return static_cast<float>(current) / static_cast<float>(max.value());
}
