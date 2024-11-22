#include "entities.hpp"

Entity::Entity(const unsigned id) : type(std::nullopt), m_id(id)
{
}

unsigned Entity::id() const
{
    return m_id;
}
