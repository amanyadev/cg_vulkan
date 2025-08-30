#include "entities/EntityManager.h"
#include <algorithm>
#include <cmath>

EntityManager::EntityManager()
    : m_treeFrequency(0.3f)
    , m_rockFrequency(0.1f) 
    , m_houseFrequency(0.02f)
    , m_lastGenerateCenter(0.0f)
    , m_lastGenerateRadius(0.0f) {
}

float EntityManager::hash21(float x, float y) const {
    float h = std::fmod(std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f, 1.0f);
    return h < 0.0f ? h + 1.0f : h;
}

float EntityManager::noise2D(float x, float y) const {
    float ix = std::floor(x);
    float iy = std::floor(y);
    float fx = x - ix;
    float fy = y - iy;
    
    // Smooth interpolation
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    
    float a = hash21(ix, iy);
    float b = hash21(ix + 1.0f, iy);
    float c = hash21(ix, iy + 1.0f);
    float d = hash21(ix + 1.0f, iy + 1.0f);
    
    return a * (1.0f - fx) * (1.0f - fy) +
           b * fx * (1.0f - fy) +
           c * (1.0f - fx) * fy +
           d * fx * fy;
}

float EntityManager::getTerrainHeight(float x, float z) const {
    // Enhanced terrain with hills and valleys
    float height = 0.0f;
    
    // Large hills
    height += std::sin(x * 0.01f) * 8.0f + std::cos(z * 0.012f) * 6.0f;
    
    // Medium rolling terrain
    height += std::sin(x * 0.03f + z * 0.025f) * 4.0f;
    
    // Small details with noise
    height += noise2D(x * 0.1f, z * 0.1f) * 3.0f;
    height += noise2D(x * 0.2f, z * 0.2f) * 1.5f;
    
    // Create valleys - areas where height is reduced
    float valley1 = std::sin(x * 0.008f) * std::cos(z * 0.008f);
    float valley2 = std::sin((x + 50.0f) * 0.006f) * std::cos((z + 30.0f) * 0.007f);
    height -= std::max(0.0f, valley1 + valley2) * 5.0f;
    
    return height;
}

bool EntityManager::shouldPlaceEntity(float x, float z, EntityType type) const {
    float terrainHeight = getTerrainHeight(x, z);
    
    // Basic terrain suitability
    if (terrainHeight < -2.0f || terrainHeight > 25.0f) return false;
    
    // Use position as seed for consistent placement
    float hash = hash21(std::floor(x / 8.0f), std::floor(z / 8.0f));
    
    switch (type) {
        case EntityType::TREE:
            // Trees prefer moderate elevations and slopes
            return hash < m_treeFrequency && terrainHeight > 0.0f && terrainHeight < 20.0f;
            
        case EntityType::ROCK:
            // Rocks can be anywhere but prefer higher elevations
            return hash < m_rockFrequency;
            
        case EntityType::HOUSE:
            // Houses prefer flatter, moderate elevation areas
            return hash < m_houseFrequency && terrainHeight > -1.0f && terrainHeight < 10.0f;
    }
    
    return false;
}

glm::vec3 EntityManager::getRandomOffset(float maxOffset) const {
    float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * maxOffset * 2.0f;
    float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * maxOffset * 2.0f;
    return glm::vec3(x, 0.0f, z);
}

bool EntityManager::isValidPlacementLocation(const glm::vec3& position, float radius) const {
    // Check if too close to existing entities
    for (const auto& entity : m_entities) {
        if (!entity->isVisible()) continue;
        
        float distance = glm::length(position - entity->getPosition());
        float minDistance = radius + entity->getBoundingRadius() + 2.0f; // 2.0f buffer
        
        if (distance < minDistance) {
            return false;
        }
    }
    return true;
}

void EntityManager::generateEntities(const glm::vec3& cameraPos, float generateRadius) {
    // Skip if we already generated around this area recently
    float distanceFromLast = glm::length(cameraPos - m_lastGenerateCenter);
    if (distanceFromLast < generateRadius * 0.5f && m_lastGenerateRadius >= generateRadius) {
        return;
    }
    
    // Clear old entities that are too far
    clearEntities();
    
    m_lastGenerateCenter = cameraPos;
    m_lastGenerateRadius = generateRadius;
    
    // Generate entities in a grid pattern around camera
    int gridSize = static_cast<int>(generateRadius / 8.0f); // 8 unit spacing
    
    for (int x = -gridSize; x <= gridSize; x++) {
        for (int z = -gridSize; z <= gridSize; z++) {
            glm::vec3 gridPos = cameraPos + glm::vec3(x * 8.0f, 0.0f, z * 8.0f);
            
            // Add some randomness to grid positions
            gridPos += getRandomOffset(3.0f);
            
            float distanceFromCamera = glm::length(gridPos - cameraPos);
            if (distanceFromCamera > generateRadius) continue;
            
            // Try to place each entity type
            for (int entityType = 0; entityType < 3; entityType++) {
                EntityType type = static_cast<EntityType>(entityType);
                
                if (shouldPlaceEntity(gridPos.x, gridPos.z, type)) {
                    // Set Y position to terrain height
                    gridPos.y = getTerrainHeight(gridPos.x, gridPos.z);
                    
                    std::unique_ptr<Entity> entity;
                    float scale = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; // 0.8 to 1.4
                    
                    switch (type) {
                        case EntityType::TREE:
                            entity = std::make_unique<TreeEntity>(gridPos, scale);
                            break;
                        case EntityType::ROCK:
                            entity = std::make_unique<RockEntity>(gridPos, scale);
                            break;
                        case EntityType::HOUSE:
                            entity = std::make_unique<HouseEntity>(gridPos, scale);
                            break;
                    }
                    
                    if (entity && isValidPlacementLocation(gridPos, entity->getBoundingRadius())) {
                        m_entities.push_back(std::move(entity));
                    }
                }
            }
        }
    }
}

void EntityManager::updateVisibility(const glm::vec3& cameraPos, float visibilityRadius) {
    for (auto& entity : m_entities) {
        float distance = glm::length(entity->getPosition() - cameraPos);
        entity->setVisible(distance <= visibilityRadius);
    }
}

void EntityManager::clearEntities() {
    m_entities.clear();
}

std::vector<Entity*> EntityManager::getVisibleEntities() const {
    std::vector<Entity*> visible;
    for (const auto& entity : m_entities) {
        if (entity->isVisible()) {
            visible.push_back(entity.get());
        }
    }
    return visible;
}

std::vector<Entity*> EntityManager::getEntitiesOfType(EntityType type) const {
    std::vector<Entity*> filtered;
    for (const auto& entity : m_entities) {
        if (entity->getType() == type && entity->isVisible()) {
            filtered.push_back(entity.get());
        }
    }
    return filtered;
}