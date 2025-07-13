#include "entities.hpp"

#include "seblib-math.hpp"

#include <utility>

namespace rl = raylib;
namespace smath = seblib::math;

namespace entities
{
AttackDetails attack_details(const Attack attack)
{
    switch (attack)
    { // NOLINTBEGIN(*magic-numbers)
    case Attack::Melee:
        return {
            .details = MeleeDetails{ rl::Vector2(18.0, 7.0) },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 34,
        };
    case Attack::Projectile:
        return {
            .details = ProjectileDetails{ 500.0 },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 25,
        };
    case Attack::Sector:
        return {
            .details = SectorDetails{ .radius = 50.0,
                                      .ang = smath::degrees_to_radians(40.0),
                                      .internal_offset = 20.0, 
                                      .external_offset = 15.0, },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 25,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace entities
