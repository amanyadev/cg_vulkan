#include "entities/RockEntity.h"
#include <algorithm>

RockEntity::RockEntity(const glm::vec3& position, float scale)
    : Entity(EntityType::ROCK, position, scale)
    , m_height(1.5f)
    , m_width(2.0f)
    , m_depth(1.8f) {
    
    // Add some variation to rock dimensions
    float heightVar = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f; // 0.5 to 1.5
    float widthVar = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.8f;  // 0.8 to 1.6
    float depthVar = 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;  // 0.7 to 1.7
    
    m_height *= heightVar;
    m_width *= widthVar;
    m_depth *= depthVar;
}

float RockEntity::getHeight() const {
    return m_height * m_scale;
}

float RockEntity::getBoundingRadius() const {
    return std::max({m_width, m_height, m_depth}) * 0.5f * m_scale;
}