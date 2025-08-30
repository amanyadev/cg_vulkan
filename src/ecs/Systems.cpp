#include "ecs/Systems.h"
#include <algorithm>
#include <cmath>

// LOD System Implementation
void LODSystem::update(ECSManager* ecs, const glm::vec3& cameraPos, const FrustumCuller& frustumCuller) {
    for (EntityID entity : m_entities) {
        auto& transform = ecs->getComponent<TransformComponent>(entity);
        auto& render = ecs->getComponent<RenderComponent>(entity);
        auto& lod = ecs->getComponent<LODComponent>(entity);
        
        // Calculate distance to camera
        float distance = glm::length(transform.position - cameraPos);
        
        // Update LOD level
        lod.updateLOD(distance);
        
        // Frustum culling
        lod.inFrustum = frustumCuller.isVisible(transform.position, render.boundingRadius);
        
        // Update render component visibility
        render.visible = lod.shouldRender();
    }
}

// Entity Generation System Implementation
float EntityGenerationSystem::hash21(const glm::vec2& p) const {
    float h = std::fmod(std::sin(p.x * 12.9898f + p.y * 78.233f) * 43758.5453f, 1.0f);
    return h < 0.0f ? h + 1.0f : h;
}

float EntityGenerationSystem::noise2D(const glm::vec2& p) const {
    glm::vec2 i = glm::floor(p);
    glm::vec2 f = p - i;
    
    // Smooth interpolation
    f = f * f * (3.0f - 2.0f * f);
    
    float a = hash21(i);
    float b = hash21(i + glm::vec2(1.0f, 0.0f));
    float c = hash21(i + glm::vec2(0.0f, 1.0f));
    float d = hash21(i + glm::vec2(1.0f, 1.0f));
    
    return glm::mix(glm::mix(a, b, f.x), glm::mix(c, d, f.x), f.y);
}

float EntityGenerationSystem::getTerrainHeight(const glm::vec2& pos) const {
    float height = 0.0f;
    
    // Large mountain ranges
    height += std::sin(pos.x * 0.008f) * 12.0f + std::cos(pos.y * 0.01f) * 10.0f;
    
    // Rolling hills
    height += std::sin(pos.x * 0.02f + pos.y * 0.015f) * 6.0f;
    height += std::cos(pos.x * 0.025f) * std::cos(pos.y * 0.03f) * 4.0f;
    
    // Fine detail
    height += noise2D(pos * 0.05f) * 3.0f;
    height += noise2D(pos * 0.1f) * 1.5f;
    
    // Create river valleys
    float river1 = std::abs(std::sin(pos.x * 0.005f + std::cos(pos.y * 0.003f) * 2.0f));
    float river2 = std::abs(std::sin(pos.y * 0.004f + std::cos(pos.x * 0.006f) * 1.5f));
    height -= glm::smoothstep(0.0f, 0.3f, 1.0f - river1) * 8.0f;
    height -= glm::smoothstep(0.0f, 0.2f, 1.0f - river2) * 6.0f;
    
    return height;
}

bool EntityGenerationSystem::isValidTreePosition(const glm::vec2& pos) const {
    glm::vec2 gridPos = glm::floor(pos / 12.0f);
    float h = hash21(gridPos);
    float terrainHeight = getTerrainHeight(pos);
    return h > 0.7f && terrainHeight > 1.0f && terrainHeight < 15.0f;
}

bool EntityGenerationSystem::isValidRockPosition(const glm::vec2& pos) const {
    glm::vec2 gridPos = glm::floor(pos / 10.0f);
    float h = hash21(gridPos * 1.7f);
    float terrainHeight = getTerrainHeight(pos);
    return h > 0.85f && terrainHeight > 5.0f;
}

bool EntityGenerationSystem::isValidHousePosition(const glm::vec2& pos) const {
    glm::vec2 gridPos = glm::floor(pos / 20.0f);
    float h = hash21(gridPos * 2.7f);
    float terrainHeight = getTerrainHeight(pos);
    return h > 0.95f && terrainHeight > -1.0f && terrainHeight < 10.0f;
}

void EntityGenerationSystem::generateEntitiesAroundCamera(ECSManager* ecs, const glm::vec3& cameraPos, float radius) {
    // Skip if we already generated around this area
    float distanceFromLast = glm::length(cameraPos - m_lastGenerationPos);
    if (distanceFromLast < radius * 0.5f && m_lastGenerationRadius >= radius) {
        return;
    }
    
    m_lastGenerationPos = cameraPos;
    m_lastGenerationRadius = radius;
    
    int gridSize = static_cast<int>(radius / 12.0f);
    
    for (int x = -gridSize; x <= gridSize; x++) {
        for (int z = -gridSize; z <= gridSize; z++) {
            glm::vec3 gridPos = cameraPos + glm::vec3(x * 12.0f, 0.0f, z * 12.0f);
            
            // Add randomness
            gridPos.x += (hash21(glm::vec2(x, z)) - 0.5f) * 8.0f;
            gridPos.z += (hash21(glm::vec2(x + 100, z + 100)) - 0.5f) * 8.0f;
            
            float distanceFromCamera = glm::length(gridPos - cameraPos);
            if (distanceFromCamera > radius) continue;
            
            // Generate trees
            if (isValidTreePosition(glm::vec2(gridPos.x, gridPos.z))) {
                EntityID entity = ecs->createEntity();
                
                gridPos.y = getTerrainHeight(glm::vec2(gridPos.x, gridPos.z));
                
                ecs->addComponent(entity, TransformComponent(gridPos));
                ecs->addComponent(entity, RenderComponent(RenderComponent::EntityType::TREE, 3.0f));
                ecs->addComponent(entity, LODComponent());
                ecs->addComponent(entity, TreeComponent());
            }
            
            // Generate rocks
            if (isValidRockPosition(glm::vec2(gridPos.x, gridPos.z))) {
                EntityID entity = ecs->createEntity();
                
                gridPos.y = getTerrainHeight(glm::vec2(gridPos.x, gridPos.z));
                
                ecs->addComponent(entity, TransformComponent(gridPos));
                ecs->addComponent(entity, RenderComponent(RenderComponent::EntityType::ROCK, 2.0f));
                ecs->addComponent(entity, LODComponent());
                ecs->addComponent(entity, RockComponent());
            }
            
            // Generate houses (less frequent)
            if (isValidHousePosition(glm::vec2(gridPos.x, gridPos.z))) {
                EntityID entity = ecs->createEntity();
                
                gridPos.y = getTerrainHeight(glm::vec2(gridPos.x, gridPos.z));
                
                ecs->addComponent(entity, TransformComponent(gridPos));
                ecs->addComponent(entity, RenderComponent(RenderComponent::EntityType::HOUSE, 5.0f));
                ecs->addComponent(entity, LODComponent());
                ecs->addComponent(entity, HouseComponent());
            }
        }
    }
}

void EntityGenerationSystem::cleanupDistantEntities(ECSManager* ecs, const glm::vec3& cameraPos, float maxDistance) {
    std::vector<EntityID> toDestroy;
    
    for (EntityID entity : m_entities) {
        auto& transform = ecs->getComponent<TransformComponent>(entity);
        float distance = glm::length(transform.position - cameraPos);
        
        if (distance > maxDistance) {
            toDestroy.push_back(entity);
        }
    }
    
    for (EntityID entity : toDestroy) {
        ecs->destroyEntity(entity);
    }
}

// Render System Implementation
std::vector<RenderSystem::RenderData> RenderSystem::getRenderList(ECSManager* ecs) {
    std::vector<RenderData> renderList;
    
    for (EntityID entity : m_entities) {
        auto& render = ecs->getComponent<RenderComponent>(entity);
        if (!render.visible) continue;
        
        auto& transform = ecs->getComponent<TransformComponent>(entity);
        auto& lod = ecs->getComponent<LODComponent>(entity);
        
        RenderData data;
        data.position = transform.position;
        data.rotation = transform.rotation;
        data.scale = transform.scale;
        data.type = render.type;
        data.lodLevel = lod.currentLOD;
        
        // Fill type-specific data
        switch (render.type) {
            case RenderComponent::EntityType::TREE: {
                auto& tree = ecs->getComponent<TreeComponent>(entity);
                data.tree.trunkHeight = tree.trunkHeight;
                data.tree.trunkRadius = tree.trunkRadius;
                data.tree.foliageRadius = tree.foliageRadius;
                break;
            }
            case RenderComponent::EntityType::ROCK: {
                auto& rock = ecs->getComponent<RockComponent>(entity);
                data.rock.dimensions = rock.dimensions;
                break;
            }
            case RenderComponent::EntityType::HOUSE: {
                auto& house = ecs->getComponent<HouseComponent>(entity);
                data.house.wallHeight = house.wallHeight;
                data.house.roofHeight = house.roofHeight;
                data.house.dimensions = house.dimensions;
                break;
            }
        }
        
        renderList.push_back(data);
    }
    
    return renderList;
}

void RenderSystem::updateVisibility(ECSManager* ecs) {
    // This is now handled by the LOD system
    // Keep this method for future visibility optimizations
}