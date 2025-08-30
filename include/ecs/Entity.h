#pragma once

#include <cstdint>
#include <bitset>

using EntityID = uint32_t;
using ComponentMask = std::bitset<32>;

class Entity {
public:
    Entity() : m_id(s_nextID++), m_componentMask(0) {}
    Entity(EntityID id) : m_id(id), m_componentMask(0) {}
    
    EntityID getID() const { return m_id; }
    
    template<typename T>
    void addComponent() {
        m_componentMask.set(static_cast<size_t>(getComponentType<T>()));
    }
    
    template<typename T>
    void removeComponent() {
        m_componentMask.reset(static_cast<size_t>(getComponentType<T>()));
    }
    
    template<typename T>
    bool hasComponent() const {
        return m_componentMask.test(static_cast<size_t>(getComponentType<T>()));
    }
    
    ComponentMask getComponentMask() const { return m_componentMask; }
    
    bool operator==(const Entity& other) const { return m_id == other.m_id; }
    bool operator!=(const Entity& other) const { return m_id != other.m_id; }
    
private:
    EntityID m_id;
    ComponentMask m_componentMask;
    static EntityID s_nextID;
    
    template<typename T>
    constexpr uint32_t getComponentType() const;
};

// Template specializations for component types
template<> constexpr uint32_t Entity::getComponentType<struct TransformComponent>() const { return 0; }
template<> constexpr uint32_t Entity::getComponentType<struct RenderComponent>() const { return 1; }
template<> constexpr uint32_t Entity::getComponentType<struct TreeComponent>() const { return 2; }
template<> constexpr uint32_t Entity::getComponentType<struct RockComponent>() const { return 3; }
template<> constexpr uint32_t Entity::getComponentType<struct HouseComponent>() const { return 4; }
template<> constexpr uint32_t Entity::getComponentType<struct LODComponent>() const { return 5; }