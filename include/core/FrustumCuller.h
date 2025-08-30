#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

struct Plane {
    glm::vec3 normal;
    float distance;
    
    Plane() : normal(0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}
    
    float distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

struct BoundingSphere {
    glm::vec3 center;
    float radius;
    
    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
};

struct Frustum {
    enum {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5
    };
    
    std::array<Plane, 6> planes;
    
    void extractFromMatrix(const glm::mat4& viewProj);
    bool isVisible(const BoundingSphere& sphere) const;
    bool isVisible(const glm::vec3& point, float radius) const;
};

class FrustumCuller {
public:
    FrustumCuller() = default;
    
    void updateFrustum(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    bool isVisible(const glm::vec3& position, float radius) const;
    
    // LOD calculation
    float calculateLOD(const glm::vec3& position, const glm::vec3& cameraPos) const;
    int getLODLevel(float distance) const;
    
private:
    Frustum m_frustum;
    
    // LOD distances
    static constexpr float LOD_DISTANCE_0 = 30.0f;   // Highest detail
    static constexpr float LOD_DISTANCE_1 = 80.0f;   // Medium detail  
    static constexpr float LOD_DISTANCE_2 = 150.0f;  // Low detail
    static constexpr float LOD_DISTANCE_3 = 250.0f;  // Lowest detail
};