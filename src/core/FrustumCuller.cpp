#include "core/FrustumCuller.h"
#include <algorithm>

void Frustum::extractFromMatrix(const glm::mat4& viewProj) {
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[LEFT].normal.x = viewProj[0][3] + viewProj[0][0];
    planes[LEFT].normal.y = viewProj[1][3] + viewProj[1][0];
    planes[LEFT].normal.z = viewProj[2][3] + viewProj[2][0];
    planes[LEFT].distance = viewProj[3][3] + viewProj[3][0];
    
    // Right plane
    planes[RIGHT].normal.x = viewProj[0][3] - viewProj[0][0];
    planes[RIGHT].normal.y = viewProj[1][3] - viewProj[1][0];
    planes[RIGHT].normal.z = viewProj[2][3] - viewProj[2][0];
    planes[RIGHT].distance = viewProj[3][3] - viewProj[3][0];
    
    // Bottom plane
    planes[BOTTOM].normal.x = viewProj[0][3] + viewProj[0][1];
    planes[BOTTOM].normal.y = viewProj[1][3] + viewProj[1][1];
    planes[BOTTOM].normal.z = viewProj[2][3] + viewProj[2][1];
    planes[BOTTOM].distance = viewProj[3][3] + viewProj[3][1];
    
    // Top plane
    planes[TOP].normal.x = viewProj[0][3] - viewProj[0][1];
    planes[TOP].normal.y = viewProj[1][3] - viewProj[1][1];
    planes[TOP].normal.z = viewProj[2][3] - viewProj[2][1];
    planes[TOP].distance = viewProj[3][3] - viewProj[3][1];
    
    // Near plane
    planes[NEAR].normal.x = viewProj[0][3] + viewProj[0][2];
    planes[NEAR].normal.y = viewProj[1][3] + viewProj[1][2];
    planes[NEAR].normal.z = viewProj[2][3] + viewProj[2][2];
    planes[NEAR].distance = viewProj[3][3] + viewProj[3][2];
    
    // Far plane
    planes[FAR].normal.x = viewProj[0][3] - viewProj[0][2];
    planes[FAR].normal.y = viewProj[1][3] - viewProj[1][2];
    planes[FAR].normal.z = viewProj[2][3] - viewProj[2][2];
    planes[FAR].distance = viewProj[3][3] - viewProj[3][2];
    
    // Normalize all planes
    for (auto& plane : planes) {
        float length = glm::length(plane.normal);
        plane.normal /= length;
        plane.distance /= length;
    }
}

bool Frustum::isVisible(const BoundingSphere& sphere) const {
    return isVisible(sphere.center, sphere.radius);
}

bool Frustum::isVisible(const glm::vec3& point, float radius) const {
    for (const auto& plane : planes) {
        float distance = plane.distanceToPoint(point);
        if (distance < -radius) {
            return false; // Completely outside this plane
        }
    }
    return true; // Inside or intersecting frustum
}

void FrustumCuller::updateFrustum(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    glm::mat4 viewProj = projMatrix * viewMatrix;
    m_frustum.extractFromMatrix(viewProj);
}

bool FrustumCuller::isVisible(const glm::vec3& position, float radius) const {
    return m_frustum.isVisible(position, radius);
}

float FrustumCuller::calculateLOD(const glm::vec3& position, const glm::vec3& cameraPos) const {
    return glm::length(position - cameraPos);
}

int FrustumCuller::getLODLevel(float distance) const {
    if (distance < LOD_DISTANCE_0) return 0; // Highest detail
    if (distance < LOD_DISTANCE_1) return 1; // Medium detail
    if (distance < LOD_DISTANCE_2) return 2; // Low detail
    if (distance < LOD_DISTANCE_3) return 3; // Lowest detail
    return 4; // Don't render (too far)
}