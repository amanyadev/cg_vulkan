#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

enum class EntityType {
    TREE,
    ROCK,
    HOUSE
};

class Entity {
public:
    Entity(EntityType type, const glm::vec3& position, float scale = 1.0f);
    virtual ~Entity() = default;

    // Getters
    EntityType getType() const { return m_type; }
    const glm::vec3& getPosition() const { return m_position; }
    float getScale() const { return m_scale; }
    float getRotation() const { return m_rotation; }
    bool isVisible() const { return m_visible; }

    // Setters
    void setPosition(const glm::vec3& position) { m_position = position; }
    void setScale(float scale) { m_scale = scale; }
    void setRotation(float rotation) { m_rotation = rotation; }
    void setVisible(bool visible) { m_visible = visible; }

    // Virtual functions for entity-specific behavior
    virtual float getHeight() const = 0;
    virtual float getBoundingRadius() const = 0;

protected:
    EntityType m_type;
    glm::vec3 m_position;
    float m_scale;
    float m_rotation;
    bool m_visible;
};