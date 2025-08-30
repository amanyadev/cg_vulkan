#pragma once

#include "entities/Entity.h"

class HouseEntity : public Entity {
public:
    HouseEntity(const glm::vec3& position, float scale = 1.0f);

    float getHeight() const override;
    float getBoundingRadius() const override;

    // House-specific properties
    float getWallHeight() const { return m_wallHeight * m_scale; }
    float getRoofHeight() const { return m_roofHeight * m_scale; }
    float getWidth() const { return m_width * m_scale; }
    float getDepth() const { return m_depth * m_scale; }

private:
    float m_wallHeight;
    float m_roofHeight;
    float m_width;
    float m_depth;
};