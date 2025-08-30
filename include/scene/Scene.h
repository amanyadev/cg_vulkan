#pragma once

#include "ecs/ECSManager.h"
#include <glm/glm.hpp>
#include <memory>

class Scene {
public:
    Scene();
    ~Scene() = default;
    
    void initialize();
    void update(float deltaTime, const glm::vec3& cameraPos);
    void cleanup();
    
    // Scene properties
    struct SceneSettings {
        // Lighting
        glm::vec3 sunDirection = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
        glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.8f);
        float sunIntensity = 3.0f;
        glm::vec3 ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
        float ambientIntensity = 0.3f;
        
        // Sky
        glm::vec3 skyColorHorizon = glm::vec3(0.9f, 0.6f, 0.4f);
        glm::vec3 skyColorZenith = glm::vec3(0.2f, 0.5f, 1.0f);
        
        // Fog
        glm::vec3 fogColor = glm::vec3(0.7f, 0.8f, 0.9f);
        float fogDensity = 0.02f;
        float fogStart = 50.0f;
        
        // Terrain
        float terrainScale = 1.0f;
        float terrainHeight = 25.0f;
        bool enableWater = true;
        float waterLevel = -1.0f;
        
        // Entities
        float entitySpawnRadius = 100.0f;
        float entityCullDistance = 150.0f;
        int maxEntities = 1000;
        bool enableTrees = true;
        bool enableRocks = true;
        bool enableHouses = true;
    };
    
    SceneSettings& getSettings() { return m_settings; }
    ECSManager* getECS() { return m_ecsManager.get(); }
    
private:
    void generateTerrain();
    void populateEntities(const glm::vec3& cameraPos);
    
    SceneSettings m_settings;
    std::unique_ptr<ECSManager> m_ecsManager;
    glm::vec3 m_lastCameraPos{0.0f};
    float m_lastSpawnTime{0.0f};
};