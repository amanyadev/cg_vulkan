#include "entities/HouseEntity.h"
#include <algorithm>

HouseEntity::HouseEntity(const glm::vec3& position, float scale)
    : Entity(EntityType::HOUSE, position, scale)
    , m_wallHeight(3.0f)
    , m_roofHeight(2.0f)
    , m_width(4.0f)
    , m_depth(5.0f) {
    
    // Add some variation to house dimensions
    float sizeVar = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; // 0.8 to 1.4
    m_width *= sizeVar;
    m_depth *= sizeVar;
    
    // Vary roof height slightly
    float roofVar = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f; // 0.8 to 1.2
    m_roofHeight *= roofVar;
}

float HouseEntity::getHeight() const {
    return (m_wallHeight + m_roofHeight) * m_scale;
}

float HouseEntity::getBoundingRadius() const {
    return std::max(m_width, m_depth) * 0.5f * m_scale;
}