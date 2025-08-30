#pragma once

#include "entities/Entity.h"
#include "entities/TreeEntity.h"
#include "entities/RockEntity.h"
#include "entities/HouseEntity.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class EntityManager {
public:
    EntityManager();
    ~EntityManager() = default;

    // Entity management
    void generateEntities(const glm::vec3& cameraPos, float generateRadius = 200.0f);
    void updateVisibility(const glm::vec3& cameraPos, float visibilityRadius = 150.0f);
    void clearEntities();

    // Getters
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }
    std::vector<Entity*> getVisibleEntities() const;
    std::vector<Entity*> getEntitiesOfType(EntityType type) const;

    // Terrain-related functions
    float getTerrainHeight(float x, float z) const;
    bool isValidPlacementLocation(const glm::vec3& position, float radius) const;

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    
    // Generation parameters
    float m_treeFrequency;
    float m_rockFrequency;
    float m_houseFrequency;
    
    // Last generation center to avoid regenerating
    glm::vec3 m_lastGenerateCenter;
    float m_lastGenerateRadius;
    
    // Helper functions
    float noise2D(float x, float y) const;
    float hash21(float x, float y) const;
    bool shouldPlaceEntity(float x, float z, EntityType type) const;
    glm::vec3 getRandomOffset(float maxOffset) const;
};