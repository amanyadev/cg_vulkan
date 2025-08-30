#include "scene/Scene.h"
#include "ecs/Systems.h"
#include <iostream>

Scene::Scene() {
    m_ecsManager = std::make_unique<ECSManager>();
}

void Scene::initialize() {
    std::cout << "Initializing scene..." << std::endl;
    
    // Register ECS components
    m_ecsManager->registerComponent<TransformComponent>();
    m_ecsManager->registerComponent<RenderComponent>();
    m_ecsManager->registerComponent<LODComponent>();
    m_ecsManager->registerComponent<TreeComponent>();
    m_ecsManager->registerComponent<RockComponent>();
    m_ecsManager->registerComponent<HouseComponent>();
    
    // Register ECS systems
    auto lodSystem = m_ecsManager->registerSystem<LODSystem>();
    auto entityGenSystem = m_ecsManager->registerSystem<EntityGenerationSystem>();
    auto renderSystem = m_ecsManager->registerSystem<RenderSystem>();
    
    // Set system signatures
    ComponentMask lodSignature;
    lodSignature.set(m_ecsManager->getComponentType<TransformComponent>());
    lodSignature.set(m_ecsManager->getComponentType<RenderComponent>());
    lodSignature.set(m_ecsManager->getComponentType<LODComponent>());
    m_ecsManager->setSystemSignature<LODSystem>(lodSignature);
    
    ComponentMask entityGenSignature;
    entityGenSignature.set(m_ecsManager->getComponentType<TransformComponent>());
    entityGenSignature.set(m_ecsManager->getComponentType<RenderComponent>());
    entityGenSignature.set(m_ecsManager->getComponentType<LODComponent>());
    m_ecsManager->setSystemSignature<EntityGenerationSystem>(entityGenSignature);
    
    ComponentMask renderSignature;
    renderSignature.set(m_ecsManager->getComponentType<TransformComponent>());
    renderSignature.set(m_ecsManager->getComponentType<RenderComponent>());
    renderSignature.set(m_ecsManager->getComponentType<LODComponent>());
    m_ecsManager->setSystemSignature<RenderSystem>(renderSignature);
    
    std::cout << "Scene initialized successfully" << std::endl;
}

void Scene::update(float deltaTime, const glm::vec3& cameraPos) {
    // Only update entities if camera moved significantly or enough time passed
    float distanceMoved = glm::length(cameraPos - m_lastCameraPos);
    m_lastSpawnTime += deltaTime;
    
    if (distanceMoved > 10.0f || m_lastSpawnTime > 2.0f) {
        populateEntities(cameraPos);
        m_lastCameraPos = cameraPos;
        m_lastSpawnTime = 0.0f;
    }
}

void Scene::cleanup() {
    if (m_ecsManager) {
        m_ecsManager.reset();
    }
}

void Scene::populateEntities(const glm::vec3& cameraPos) {
    auto entityGenSystem = m_ecsManager->getSystem<EntityGenerationSystem>();
    if (entityGenSystem) {
        // Generate entities around camera
        entityGenSystem->generateEntitiesAroundCamera(
            m_ecsManager.get(), 
            cameraPos, 
            m_settings.entitySpawnRadius
        );
        
        // Cleanup distant entities
        entityGenSystem->cleanupDistantEntities(
            m_ecsManager.get(), 
            cameraPos, 
            m_settings.entityCullDistance
        );
    }
}