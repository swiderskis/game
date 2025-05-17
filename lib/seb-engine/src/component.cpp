#include "seb-engine-ecs.hpp"

namespace seb_engine
{
void Components::uninit_destroyed_entity(const unsigned id)
{
    for (auto& [_, component] : m_components)
    {
        component->reset(id);
    }
}

EntityComponents Components::by_id(const unsigned id)
{
    return { *this, id };
}

EntityComponents::EntityComponents(Components& components, unsigned id) : m_components(&components), m_id(id)
{
}
} // namespace seb_engine
