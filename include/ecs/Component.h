#pragma once

#include <glm/glm.hpp>
#include <cstdint>

// Component types
enum class ComponentType : uint32_t {
    TRANSFORM = 1 << 0,
    RENDER = 1 << 1,
    TREE = 1 << 2,
    ROCK = 1 << 3,
    HOUSE = 1 << 4,
    LOD = 1 << 5
};

// Base component interface
struct Component {
    virtual ~Component() = default;
};

// Transform component
struct TransformComponent : public Component {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    
    TransformComponent() = default;
    TransformComponent(const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl) {}
};

// Render component
struct RenderComponent : public Component {
    enum class EntityType {
        TREE,
        ROCK,
        HOUSE
    };
    
    EntityType type{EntityType::TREE};
    bool visible{true};
    float boundingRadius{1.0f};
    
    RenderComponent() = default;
    RenderComponent(EntityType t, float radius) : type(t), boundingRadius(radius) {}
};

// LOD component
struct LODComponent : public Component {
    int currentLOD{0};
    float distance{0.0f};
    bool inFrustum{true};
    
    static constexpr float LOD_DISTANCES[4] = {30.0f, 80.0f, 150.0f, 250.0f};
    
    void updateLOD(float dist) {
        distance = dist;
        if (dist < LOD_DISTANCES[0]) currentLOD = 0;
        else if (dist < LOD_DISTANCES[1]) currentLOD = 1;
        else if (dist < LOD_DISTANCES[2]) currentLOD = 2;
        else if (dist < LOD_DISTANCES[3]) currentLOD = 3;
        else currentLOD = 4; // Don't render
    }
    
    bool shouldRender() const { return inFrustum && currentLOD < 4; }
};

// Tree-specific component
struct TreeComponent : public Component {
    float trunkHeight{3.0f};
    float trunkRadius{0.25f};
    float foliageRadius{2.0f};
    
    TreeComponent() {
        // Add variation
        float variation = 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
        trunkHeight *= variation;
        foliageRadius *= variation;
    }
};

// Rock-specific component
struct RockComponent : public Component {
    glm::vec3 dimensions{1.2f, 1.0f, 1.5f};
    
    RockComponent() {
        // Add variation
        dimensions.x *= 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
        dimensions.y *= 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;
        dimensions.z *= 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 0.8f;
    }
};

// House-specific component
struct HouseComponent : public Component {
    float wallHeight{3.0f};
    float roofHeight{2.0f};
    glm::vec2 dimensions{4.0f, 5.0f};
    
    HouseComponent() {
        float sizeVar = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
        dimensions *= sizeVar;
        roofHeight *= 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f;
    }
};