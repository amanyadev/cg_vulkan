#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class OrbitCamera {
public:
    OrbitCamera();
    ~OrbitCamera() = default;
    
    // Camera controls
    void orbit(float deltaYaw, float deltaPitch);
    void pan(float deltaX, float deltaY);
    void zoom(float deltaZoom);
    void reset();
    void lookAt(const glm::vec3& center, float radius);
    
    // Smooth transitions
    void update(float deltaTime);
    void setSmoothing(float smoothFactor) { m_smoothFactor = smoothFactor; }
    
    // Auto rotation
    void setAutoRotate(bool enabled, float speed = 1.0f);
    
    // Getters
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    glm::vec3 getPosition() const;
    glm::vec3 getDirection() const;
    glm::vec3 getUp() const { return glm::vec3(0.0f, 1.0f, 0.0f); }
    glm::vec3 getRight() const;
    
    // Camera parameters
    void setFOV(float fov) { m_fov = fov; }
    void setNearFar(float near, float far) { m_near = near; m_far = far; }
    void setTarget(const glm::vec3& target) { m_target = target; }
    void setDistance(float distance) { m_targetDistance = distance; }
    
    float getFOV() const { return m_fov; }
    float getNear() const { return m_near; }
    float getFar() const { return m_far; }
    glm::vec3 getTarget() const { return m_target; }
    float getDistance() const { return m_distance; }
    
    // Limits
    void setPitchLimits(float minPitch, float maxPitch) { 
        m_minPitch = minPitch; 
        m_maxPitch = maxPitch; 
    }
    void setDistanceLimits(float minDistance, float maxDistance) {
        m_minDistance = minDistance;
        m_maxDistance = maxDistance;
    }
    
private:
    void updatePosition();
    void clampAngles();
    void clampDistance();
    
    // Camera parameters
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    float m_distance{5.0f};
    float m_yaw{0.0f};        // Rotation around Y axis
    float m_pitch{0.0f};      // Rotation around X axis
    
    // Target values for smooth interpolation
    glm::vec3 m_targetTarget{0.0f, 0.0f, 0.0f};
    float m_targetDistance{5.0f};
    float m_targetYaw{0.0f};
    float m_targetPitch{0.0f};
    
    // Camera properties
    float m_fov{45.0f};
    float m_near{0.1f};
    float m_far{1000.0f};
    
    // Limits
    float m_minPitch{-89.0f};
    float m_maxPitch{89.0f};
    float m_minDistance{0.5f};
    float m_maxDistance{100.0f};
    
    // Smooth interpolation
    float m_smoothFactor{8.0f};
    
    // Auto rotation
    bool m_autoRotate{false};
    float m_autoRotateSpeed{1.0f};
    
    // Sensitivity
    float m_orbitSensitivity{1.0f};
    float m_panSensitivity{1.0f};
    float m_zoomSensitivity{1.0f};
};