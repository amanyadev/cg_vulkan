#pragma once

#include "ecs/Entity.h"
#include "ecs/Component.h"
#include "ecs/ComponentManager.h"
#include "core/FrustumCuller.h"
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

class System {
public:
    virtual ~System() = default;
    std::set<EntityID> m_entities;
};

class ECSManager {
public:
    ECSManager();
    ~ECSManager() = default;
    
    // Entity management
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    
    // Component management
    template<typename T>
    void registerComponent() {
        m_componentManager->registerComponent<T>();
    }
    
    template<typename T>
    void addComponent(EntityID entity, T component) {
        m_componentManager->addComponent<T>(entity, component);
        
        auto& entityMask = m_entityMasks[entity];
        entityMask.set(m_componentManager->getComponentType<T>(), true);
        
        updateEntitySystems(entity);
    }
    
    template<typename T>
    void removeComponent(EntityID entity) {
        m_componentManager->removeComponent<T>(entity);
        
        auto& entityMask = m_entityMasks[entity];
        entityMask.set(m_componentManager->getComponentType<T>(), false);
        
        updateEntitySystems(entity);
    }
    
    template<typename T>
    T& getComponent(EntityID entity) {
        return m_componentManager->getComponent<T>(entity);
    }
    
    template<typename T>
    uint32_t getComponentType() {
        return m_componentManager->getComponentType<T>();
    }
    
    // System management
    template<typename T>
    std::shared_ptr<T> registerSystem() {
        std::type_index typeName = std::type_index(typeid(T));
        
        auto system = std::make_shared<T>();
        m_systems[typeName] = system;
        return system;
    }
    
    template<typename T>
    void setSystemSignature(ComponentMask signature) {
        std::type_index typeName = std::type_index(typeid(T));
        m_signatures[typeName] = signature;
    }
    
    template<typename T>
    std::shared_ptr<T> getSystem() {
        std::type_index typeName = std::type_index(typeid(T));
        return std::static_pointer_cast<T>(m_systems[typeName]);
    }
    
    // Utility functions
    std::vector<EntityID> getEntitiesWithComponents(ComponentMask mask);
    void updateEntitySystems(EntityID entity);
    
    // Get component arrays for iteration
    template<typename T>
    std::shared_ptr<TypedComponentArray<T>> getComponentArray() {
        return m_componentManager->getComponentArray<T>();
    }
    
private:
    std::unique_ptr<ComponentManager> m_componentManager;
    std::unordered_map<EntityID, ComponentMask> m_entityMasks;
    std::unordered_map<std::type_index, ComponentMask> m_signatures;
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems;
    std::set<EntityID> m_availableEntityIDs;
    EntityID m_nextEntityID{0};
    static constexpr EntityID MAX_ENTITIES = 10000;
};