#pragma once

#include "ecs/Component.h"
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <vector>

class ComponentArray {
public:
    virtual ~ComponentArray() = default;
    virtual void entityDestroyed(EntityID entity) = 0;
};

template<typename T>
class TypedComponentArray : public ComponentArray {
public:
    void insertComponent(EntityID entity, T component) {
        size_t newIndex = m_size;
        m_entityToIndex[entity] = newIndex;
        m_indexToEntity[newIndex] = entity;
        m_componentArray[newIndex] = component;
        ++m_size;
    }
    
    void removeComponent(EntityID entity) {
        size_t indexOfRemovedEntity = m_entityToIndex[entity];
        size_t indexOfLastElement = m_size - 1;
        
        // Move last element to deleted element's place
        m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];
        
        EntityID entityOfLastElement = m_indexToEntity[indexOfLastElement];
        m_entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
        m_indexToEntity[indexOfRemovedEntity] = entityOfLastElement;
        
        m_entityToIndex.erase(entity);
        m_indexToEntity.erase(indexOfLastElement);
        
        --m_size;
    }
    
    T& getComponent(EntityID entity) {
        return m_componentArray[m_entityToIndex[entity]];
    }
    
    void entityDestroyed(EntityID entity) override {
        if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
            removeComponent(entity);
        }
    }
    
    std::vector<std::pair<EntityID, T*>> getAllComponents() {
        std::vector<std::pair<EntityID, T*>> result;
        for (size_t i = 0; i < m_size; ++i) {
            result.emplace_back(m_indexToEntity[i], &m_componentArray[i]);
        }
        return result;
    }
    
private:
    static constexpr size_t MAX_ENTITIES = 10000;
    std::array<T, MAX_ENTITIES> m_componentArray{};
    std::unordered_map<EntityID, size_t> m_entityToIndex{};
    std::unordered_map<size_t, EntityID> m_indexToEntity{};
    size_t m_size{0};
};

class ComponentManager {
public:
    template<typename T>
    void registerComponent() {
        std::type_index typeName = std::type_index(typeid(T));
        m_componentTypes[typeName] = m_nextComponentType;
        m_componentArrays[typeName] = std::make_shared<TypedComponentArray<T>>();
        ++m_nextComponentType;
    }
    
    template<typename T>
    uint32_t getComponentType() {
        std::type_index typeName = std::type_index(typeid(T));
        return m_componentTypes[typeName];
    }
    
    template<typename T>
    void addComponent(EntityID entity, T component) {
        getComponentArray<T>()->insertComponent(entity, component);
    }
    
    template<typename T>
    void removeComponent(EntityID entity) {
        getComponentArray<T>()->removeComponent(entity);
    }
    
    template<typename T>
    T& getComponent(EntityID entity) {
        return getComponentArray<T>()->getComponent(entity);
    }
    
    template<typename T>
    std::shared_ptr<TypedComponentArray<T>> getComponentArray() {
        std::type_index typeName = std::type_index(typeid(T));
        return std::static_pointer_cast<TypedComponentArray<T>>(m_componentArrays[typeName]);
    }
    
    void entityDestroyed(EntityID entity) {
        for (auto const& pair : m_componentArrays) {
            auto const& component = pair.second;
            component->entityDestroyed(entity);
        }
    }
    
private:
    std::unordered_map<std::type_index, uint32_t> m_componentTypes{};
    std::unordered_map<std::type_index, std::shared_ptr<ComponentArray>> m_componentArrays{};
    uint32_t m_nextComponentType{0};
};