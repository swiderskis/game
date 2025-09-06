#include "entities.hpp"

#include "sl-math.hpp"

#include <utility>

namespace sm = seblib::math;

namespace entities
{
auto attack_details(const Attack attack) -> AttackDetails
{
    switch (attack)
    { // NOLINTBEGIN(*magic-numbers)
    case Attack::Melee:
        return {
            .details = MeleeDetails{ { 18.0, 7.0 } },
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
                                      .angle = sm::degrees_to_radians(40.0),
                                      .line_offset = 20.0, 
                                      .sector_offset = 15.0, },
            .lifespan = 0.3,
            .delay = 0.0,
            .cooldown = 0.5,
            .damage = 25,
        };
    } // NOLINTEND(*magic-numbers)

    std::unreachable();
}
} // namespace entities
