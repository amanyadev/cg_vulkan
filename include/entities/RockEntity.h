#pragma once

#include "entities/Entity.h"

class RockEntity : public Entity {
public:
    RockEntity(const glm::vec3& position, float scale = 1.0f);

    float getHeight() const override;
    float getBoundingRadius() const override;

    // Rock-specific properties
    float getWidth() const { return m_width * m_scale; }
    float getDepth() const { return m_depth * m_scale; }

private:
    float m_height;
    float m_width;
    float m_depth;
};