#pragma once

#include "ecs/ECSManager.h"
#include "ecs/Component.h"
#include "core/FrustumCuller.h"
#include <glm/glm.hpp>

// Forward declarations
class ECSManager;

// LOD and Culling System
class LODSystem : public System {
public:
    void update(ECSManager* ecs, const glm::vec3& cameraPos, const FrustumCuller& frustumCuller);
};

// Entity Generation System
class EntityGenerationSystem : public System {
public:
    void generateEntitiesAroundCamera(ECSManager* ecs, const glm::vec3& cameraPos, float radius = 200.0f);
    void cleanupDistantEntities(ECSManager* ecs, const glm::vec3& cameraPos, float maxDistance = 300.0f);
    
private:
    glm::vec3 m_lastGenerationPos{0.0f};
    float m_lastGenerationRadius{0.0f};
    
    float getTerrainHeight(const glm::vec2& pos) const;
    bool isValidTreePosition(const glm::vec2& pos) const;
    bool isValidRockPosition(const glm::vec2& pos) const;
    bool isValidHousePosition(const glm::vec2& pos) const;
    
    float hash21(const glm::vec2& p) const;
    float noise2D(const glm::vec2& p) const;
};

// Render System (for CPU-side render list generation)
class RenderSystem : public System {
public:
    struct RenderData {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        RenderComponent::EntityType type;
        int lodLevel;
        
        // Type-specific data
        union {
            struct { float trunkHeight, trunkRadius, foliageRadius; } tree;
            struct { glm::vec3 dimensions; } rock;
            struct { float wallHeight, roofHeight; glm::vec2 dimensions; } house;
        };
    };
    
    std::vector<RenderData> getRenderList(ECSManager* ecs);
    void updateVisibility(ECSManager* ecs);
};