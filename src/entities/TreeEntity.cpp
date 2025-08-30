#include "entities/TreeEntity.h"
#include <algorithm>

TreeEntity::TreeEntity(const glm::vec3& position, float scale)
    : Entity(EntityType::TREE, position, scale)
    , m_trunkHeight(4.0f)
    , m_trunkRadius(0.3f)
    , m_foliageRadius(2.5f) {
    
    // Add some variation to tree dimensions
    float variation = 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; // 0.7 to 1.3
    m_trunkHeight *= variation;
    m_foliageRadius *= variation;
}

float TreeEntity::getHeight() const {
    return (m_trunkHeight + m_foliageRadius) * m_scale;
}

float TreeEntity::getBoundingRadius() const {
    return std::max(m_trunkRadius, m_foliageRadius) * m_scale;
}