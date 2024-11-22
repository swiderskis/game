#include "entities.hpp"

unsigned Entity::id() const
{
    return m_id;
}

Entity::Entity(const unsigned id) : type(std::nullopt), m_id(id)
{
}
