#include "entities.hpp"

Entity::Entity(unsigned id) : m_id(id), m_type(std::nullopt) {};

unsigned Entity::id() const
{
    return m_id;
}

std::optional<EntityType> Entity::type() const
{
    return m_type;
}
