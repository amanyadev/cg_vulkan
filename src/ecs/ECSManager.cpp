#include "ecs/ECSManager.h"
#include <typeindex>

ECSManager::ECSManager() {
    m_componentManager = std::make_unique<ComponentManager>();
}

EntityID ECSManager::createEntity() {
    EntityID id;
    
    if (!m_availableEntityIDs.empty()) {
        id = *m_availableEntityIDs.begin();
        m_availableEntityIDs.erase(m_availableEntityIDs.begin());
    } else {
        id = m_nextEntityID++;
    }
    
    m_entityMasks[id] = ComponentMask();
    return id;
}

void ECSManager::destroyEntity(EntityID entity) {
    m_entityMasks.erase(entity);
    
    m_componentManager->entityDestroyed(entity);
    
    for (auto const& pair : m_systems) {
        auto const& system = pair.second;
        system->m_entities.erase(entity);
    }
    
    m_availableEntityIDs.insert(entity);
}

std::vector<EntityID> ECSManager::getEntitiesWithComponents(ComponentMask mask) {
    std::vector<EntityID> entities;
    
    for (auto const& pair : m_entityMasks) {
        EntityID entity = pair.first;
        ComponentMask entityMask = pair.second;
        
        if ((entityMask & mask) == mask) {
            entities.push_back(entity);
        }
    }
    
    return entities;
}

void ECSManager::updateEntitySystems(EntityID entity) {
    ComponentMask entityMask = m_entityMasks[entity];
    
    for (auto const& pair : m_systems) {
        auto const& type = pair.first;
        auto const& system = pair.second;
        auto const& systemSignature = m_signatures[type];
        
        if ((entityMask & systemSignature) == systemSignature) {
            system->m_entities.insert(entity);
        } else {
            system->m_entities.erase(entity);
        }
    }
}