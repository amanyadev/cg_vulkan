#include "viewer/Gizmo.h"
#include <iostream>

Gizmo::Gizmo(VulkanDevice* device) : m_device(device) {}

Gizmo::~Gizmo() {
    cleanup();
}

void Gizmo::initialize() {
    std::cout << "Initializing Gizmo..." << std::endl;
    
    // Create geometry for different modes
    createGeometry();
    
    // Create pipeline
    // createPipeline(); // Will implement later
    
    std::cout << "Gizmo initialized" << std::endl;
}

void Gizmo::render(VkCommandBuffer commandBuffer, const OrbitCamera& camera, const glm::mat4& modelMatrix) {
    if (!m_enabled) return;
    
    // Update uniform buffer
    updateUniformBuffer(camera, modelMatrix);
    
    // Render gizmo geometry
    // Implementation will come later
}

void Gizmo::cleanup() {
    // Cleanup Vulkan resources
    if (m_vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getDevice(), m_vertexBuffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_vertexBufferMemory, nullptr);
    }
    
    if (m_indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getDevice(), m_indexBuffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_indexBufferMemory, nullptr);
    }
    
    if (m_uniformBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getDevice(), m_uniformBuffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_uniformMemory, nullptr);
    }
    
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
    }
    
    if (m_descriptorLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device->getDevice(), m_descriptorLayout, nullptr);
    }
    
    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->getDevice(), m_pipeline, nullptr);
    }
    
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->getDevice(), m_pipelineLayout, nullptr);
    }
}

bool Gizmo::handleMouseClick(double mouseX, double mouseY, const OrbitCamera& camera, 
                            const glm::mat4& modelMatrix, uint32_t screenWidth, uint32_t screenHeight) {
    if (!m_enabled) return false;
    
    // Convert mouse position to world ray
    glm::vec3 rayOrigin = screenToWorld(mouseX, mouseY, camera, screenWidth, screenHeight);
    glm::vec3 rayDirection = camera.getDirection(); // Simplified for now
    
    // Test intersection with gizmo axes
    float distance;
    GizmoAxis hitAxis = pickAxis(rayOrigin, rayDirection, modelMatrix, distance);
    
    if (hitAxis != GizmoAxis::NONE) {
        m_activeAxis = hitAxis;
        m_isDragging = true;
        m_initialTransform = modelMatrix;
        return true;
    }
    
    return false;
}

glm::mat4 Gizmo::handleMouseDrag(double mouseX, double mouseY, double deltaX, double deltaY,
                                const OrbitCamera& camera, const glm::mat4& modelMatrix,
                                uint32_t screenWidth, uint32_t screenHeight) {
    if (!m_isDragging || m_activeAxis == GizmoAxis::NONE) {
        return modelMatrix;
    }
    
    // Apply transformation based on active axis and mode
    glm::mat4 transform = modelMatrix;
    
    switch (m_mode) {
        case GizmoMode::TRANSLATE: {
            float sensitivity = 0.01f;
            glm::vec3 translation(0.0f);
            
            if ((m_activeAxis & GizmoAxis::X) != GizmoAxis::NONE) translation.x += static_cast<float>(deltaX * sensitivity);
            if ((m_activeAxis & GizmoAxis::Y) != GizmoAxis::NONE) translation.y += static_cast<float>(-deltaY * sensitivity);
            if ((m_activeAxis & GizmoAxis::Z) != GizmoAxis::NONE) translation.z += static_cast<float>(deltaY * sensitivity);
            
            transform = glm::translate(transform, translation);
            break;
        }
        case GizmoMode::ROTATE: {
            float sensitivity = 0.01f;
            
            if ((m_activeAxis & GizmoAxis::X) != GizmoAxis::NONE) {
                transform = glm::rotate(transform, static_cast<float>(deltaY * sensitivity), glm::vec3(1,0,0));
            }
            if ((m_activeAxis & GizmoAxis::Y) != GizmoAxis::NONE) {
                transform = glm::rotate(transform, static_cast<float>(deltaX * sensitivity), glm::vec3(0,1,0));
            }
            if ((m_activeAxis & GizmoAxis::Z) != GizmoAxis::NONE) {
                transform = glm::rotate(transform, static_cast<float>(deltaX * sensitivity), glm::vec3(0,0,1));
            }
            break;
        }
        case GizmoMode::SCALE: {
            float sensitivity = 0.01f;
            glm::vec3 scale(1.0f);
            
            float scaleFactor = 1.0f + static_cast<float>((deltaX + deltaY) * sensitivity);
            
            if ((m_activeAxis & GizmoAxis::X) != GizmoAxis::NONE) scale.x = scaleFactor;
            if ((m_activeAxis & GizmoAxis::Y) != GizmoAxis::NONE) scale.y = scaleFactor;
            if ((m_activeAxis & GizmoAxis::Z) != GizmoAxis::NONE) scale.z = scaleFactor;
            
            transform = glm::scale(transform, scale);
            break;
        }
    }
    
    return transform;
}

void Gizmo::endInteraction() {
    m_activeAxis = GizmoAxis::NONE;
    m_isDragging = false;
}

// Private methods (stubs)
void Gizmo::createGeometry() {
    createTranslationGizmo();
}

void Gizmo::createPipeline() {
    // Will implement pipeline creation
}

void Gizmo::updateUniformBuffer(const OrbitCamera& camera, const glm::mat4& modelMatrix) {
    // Will implement uniform buffer updates
}

GizmoAxis Gizmo::pickAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                         const glm::mat4& gizmoMatrix, float& distance) {
    // Simplified axis picking - will implement proper ray-geometry intersection
    distance = 0.0f;
    return GizmoAxis::NONE;
}

glm::vec3 Gizmo::screenToWorld(double mouseX, double mouseY, const OrbitCamera& camera,
                              uint32_t screenWidth, uint32_t screenHeight) {
    // Convert screen coordinates to world ray
    // Simplified implementation for now
    return camera.getPosition();
}

void Gizmo::createTranslationGizmo() {
    m_vertices.clear();
    m_colors.clear();
    m_indices.clear();
    
    // Create simple line-based translation gizmo
    // X axis (red)
    m_vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    m_vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    m_colors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    m_colors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Y axis (green)
    m_vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    m_vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    m_colors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    m_colors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Z axis (blue)
    m_vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    m_vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    m_colors.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    m_colors.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Indices for lines
    for (uint32_t i = 0; i < 6; ++i) {
        m_indices.push_back(i);
    }
    
    m_vertexCount = static_cast<uint32_t>(m_vertices.size());
    m_indexCount = static_cast<uint32_t>(m_indices.size());
}

void Gizmo::createRotationGizmo() {
    // Will implement rotation gizmo geometry
}

void Gizmo::createScaleGizmo() {
    // Will implement scale gizmo geometry
}