#include "viewer/OrbitCamera.h"
#include <algorithm>
#include <cmath>

OrbitCamera::OrbitCamera() {
    reset();
}

void OrbitCamera::orbit(float deltaYaw, float deltaPitch) {
    m_targetYaw += deltaYaw * m_orbitSensitivity;
    m_targetPitch += deltaPitch * m_orbitSensitivity;
    clampAngles();
}

void OrbitCamera::pan(float deltaX, float deltaY) {
    glm::vec3 right = getRight();
    glm::vec3 up = getUp();
    
    // Pan relative to current distance for consistent feel
    float panSpeed = m_distance * 0.001f * m_panSensitivity;
    m_targetTarget += right * (-deltaX * panSpeed) + up * (deltaY * panSpeed);
}

void OrbitCamera::zoom(float deltaZoom) {
    m_targetDistance *= (1.0f + deltaZoom * m_zoomSensitivity * 0.1f);
    clampDistance();
}

void OrbitCamera::reset() {
    m_target = m_targetTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_distance = m_targetDistance = 5.0f;
    m_yaw = m_targetYaw = 0.0f;
    m_pitch = m_targetPitch = 0.0f;
}

void OrbitCamera::lookAt(const glm::vec3& center, float radius) {
    m_targetTarget = center;
    m_targetDistance = radius * 2.5f; // Nice viewing distance
    clampDistance();
}

void OrbitCamera::update(float deltaTime) {
    // Smooth interpolation
    float t = 1.0f - exp(-m_smoothFactor * deltaTime);
    
    m_target = glm::mix(m_target, m_targetTarget, t);
    m_distance = glm::mix(m_distance, m_targetDistance, t);
    m_yaw = glm::mix(m_yaw, m_targetYaw, t);
    m_pitch = glm::mix(m_pitch, m_targetPitch, t);
    
    // Auto rotation
    if (m_autoRotate) {
        m_targetYaw += m_autoRotateSpeed * deltaTime;
        m_yaw += m_autoRotateSpeed * deltaTime;
    }
    
    updatePosition();
}

void OrbitCamera::setAutoRotate(bool enabled, float speed) {
    m_autoRotate = enabled;
    m_autoRotateSpeed = speed;
}

glm::mat4 OrbitCamera::getViewMatrix() const {
    glm::vec3 position = getPosition();
    return glm::lookAt(position, m_target, getUp());
}

glm::mat4 OrbitCamera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_near, m_far);
}

glm::vec3 OrbitCamera::getPosition() const {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    
    float x = cos(pitchRad) * sin(yawRad);
    float y = sin(pitchRad);
    float z = cos(pitchRad) * cos(yawRad);
    
    return m_target + glm::vec3(x, y, z) * m_distance;
}

glm::vec3 OrbitCamera::getDirection() const {
    return glm::normalize(m_target - getPosition());
}

glm::vec3 OrbitCamera::getRight() const {
    return glm::normalize(glm::cross(getDirection(), getUp()));
}

void OrbitCamera::updatePosition() {
    // Position is calculated in getPosition()
}

void OrbitCamera::clampAngles() {
    m_targetPitch = glm::clamp(m_targetPitch, m_minPitch, m_maxPitch);
    
    // Keep yaw in reasonable range
    while (m_targetYaw > 360.0f) m_targetYaw -= 360.0f;
    while (m_targetYaw < -360.0f) m_targetYaw += 360.0f;
}

void OrbitCamera::clampDistance() {
    m_targetDistance = glm::clamp(m_targetDistance, m_minDistance, m_maxDistance);
}