#pragma once

#include "entities/Entity.h"

class TreeEntity : public Entity {
public:
    TreeEntity(const glm::vec3& position, float scale = 1.0f);

    float getHeight() const override;
    float getBoundingRadius() const override;

    // Tree-specific properties
    float getTrunkHeight() const { return m_trunkHeight * m_scale; }
    float getTrunkRadius() const { return m_trunkRadius * m_scale; }
    float getFoliageRadius() const { return m_foliageRadius * m_scale; }

private:
    float m_trunkHeight;
    float m_trunkRadius;
    float m_foliageRadius;
};