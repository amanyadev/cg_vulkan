#include "entities/Entity.h"

Entity::Entity(EntityType type, const glm::vec3& position, float scale)
    : m_type(type), m_position(position), m_scale(scale), m_rotation(0.0f), m_visible(true) {
}