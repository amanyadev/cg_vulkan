#pragma once

#include "core/VulkanDevice.h"
#include "viewer/OrbitCamera.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

enum class GizmoMode {
    TRANSLATE,
    ROTATE,
    SCALE
};

enum class GizmoAxis {
    NONE = 0,
    X = 1,
    Y = 2,
    Z = 4,
    XY = X | Y,
    XZ = X | Z,
    YZ = Y | Z,
    XYZ = X | Y | Z
};

// Enable bitwise operations for GizmoAxis
inline GizmoAxis operator&(GizmoAxis a, GizmoAxis b) {
    return static_cast<GizmoAxis>(static_cast<int>(a) & static_cast<int>(b));
}

inline GizmoAxis operator|(GizmoAxis a, GizmoAxis b) {
    return static_cast<GizmoAxis>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator!=(GizmoAxis a, GizmoAxis b) {
    return static_cast<int>(a) != static_cast<int>(b);
}

class Gizmo {
public:
    Gizmo(VulkanDevice* device);
    ~Gizmo();
    
    void initialize();
    void render(VkCommandBuffer commandBuffer, const OrbitCamera& camera, const glm::mat4& modelMatrix);
    void cleanup();
    
    // Interaction
    bool handleMouseClick(double mouseX, double mouseY, const OrbitCamera& camera, 
                         const glm::mat4& modelMatrix, uint32_t screenWidth, uint32_t screenHeight);
    glm::mat4 handleMouseDrag(double mouseX, double mouseY, double deltaX, double deltaY,
                             const OrbitCamera& camera, const glm::mat4& modelMatrix,
                             uint32_t screenWidth, uint32_t screenHeight);
    void endInteraction();
    
    // Settings
    void setMode(GizmoMode mode) { m_mode = mode; }
    void setSize(float size) { m_size = size; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    GizmoMode getMode() const { return m_mode; }
    bool isEnabled() const { return m_enabled; }
    bool isInteracting() const { return m_activeAxis != GizmoAxis::NONE; }
    
private:
    void createGeometry();
    void createPipeline();
    void updateUniformBuffer(const OrbitCamera& camera, const glm::mat4& modelMatrix);
    
    // Ray-gizmo intersection
    GizmoAxis pickAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                      const glm::mat4& gizmoMatrix, float& distance);
    glm::vec3 screenToWorld(double mouseX, double mouseY, const OrbitCamera& camera,
                           uint32_t screenWidth, uint32_t screenHeight);
    
    // Geometry creation helpers
    void createTranslationGizmo();
    void createRotationGizmo();
    void createScaleGizmo();
    
    VulkanDevice* m_device;
    
    // Vulkan resources
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    VkBuffer m_vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_vertexBufferMemory{VK_NULL_HANDLE};
    VkBuffer m_indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_indexBufferMemory{VK_NULL_HANDLE};
    VkBuffer m_uniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_uniformMemory{VK_NULL_HANDLE};
    void* m_uniformMapped{nullptr};
    
    VkDescriptorSetLayout m_descriptorLayout{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
    
    // Gizmo state
    GizmoMode m_mode{GizmoMode::TRANSLATE};
    GizmoAxis m_activeAxis{GizmoAxis::NONE};
    GizmoAxis m_hoveredAxis{GizmoAxis::NONE};
    bool m_enabled{true};
    float m_size{1.0f};
    
    // Geometry data
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_colors;
    std::vector<uint32_t> m_indices;
    uint32_t m_vertexCount{0};
    uint32_t m_indexCount{0};
    
    // Interaction state
    glm::vec3 m_lastIntersection{0.0f};
    glm::mat4 m_initialTransform{1.0f};
    bool m_isDragging{false};
};