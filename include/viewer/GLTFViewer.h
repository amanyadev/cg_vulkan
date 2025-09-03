#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include "viewer/GLTFLoader.h"
#include "viewer/OrbitCamera.h"
#include "viewer/Gizmo.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

struct ViewerSettings {
    // Rendering modes
    int renderMode = 0; // 0=PBR, 1=Wireframe, 2=Points, 3=Normals, 4=Albedo, 5=Metallic, 6=Roughness, 7=AO
    
    // Primary light
    glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.8f, -0.3f));
    glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.8f);
    float lightIntensity = 3.0f;
    
    // Secondary light
    glm::vec3 light2Direction = glm::normalize(glm::vec3(0.3f, -0.6f, 0.7f));
    glm::vec3 light2Color = glm::vec3(0.4f, 0.6f, 1.0f);
    float light2Intensity = 1.0f;
    
    // Ambient lighting
    glm::vec3 ambientColor = glm::vec3(0.3f, 0.4f, 0.6f);
    float ambientIntensity = 0.3f;
    
    // IBL and environment
    float iblIntensity = 1.0f;
    float shadowIntensity = 1.0f;
    
    // Post-processing
    float exposure = 1.0f;
    float gamma = 2.2f;
    
    // Material properties
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    
    // Material overrides (legacy)
    bool useVertexColors = true;
    bool showTextures = true;
    bool showNormals = false;
    glm::vec3 materialColor = glm::vec3(0.8f, 0.8f, 0.8f);
    float metallic = 0.0f;  // Deprecated in favor of metallicFactor
    float roughness = 0.5f; // Deprecated in favor of roughnessFactor
    
    // Debug visualization
    bool showWireframe = false;
    bool showBoundingBox = false;
    bool showGizmo = true;
    
    // Camera
    bool enableAutoRotate = false;
    float autoRotateSpeed = 0.5f;
    
    // Performance
    bool enableVSync = true;
    bool showFPS = true;
};

class GLTFViewer {
public:
    GLTFViewer(VulkanDevice* device, SwapChain* swapChain);
    ~GLTFViewer();
    
    void initialize();
    void loadModel(const std::string& filePath);
    void update(float deltaTime);
    void render();
    void renderToCommandBuffer(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
    void cleanup();
    
    // Camera controls
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);
    
    // Screenshot functionality
    void takeScreenshot(const std::string& filePath);
    
    // UI integration
    void renderUI();
    
    // Getters for GUI
    bool isModelLoaded() const { return m_modelLoaded; }
    bool hasModel() const { return m_modelLoaded; }
    std::string getModelPath() const { return m_modelPath; }
    glm::vec3 getModelCenter() const { return m_modelCenter; }
    float getModelRadius() const { return m_modelRadius; }
    uint32_t getVertexCount() const { return m_loader ? m_loader->getVertexCount() : 0; }
    uint32_t getTriangleCount() const { return m_loader ? m_loader->getTriangleCount() : 0; }
    uint32_t getMeshCount() const { return m_loader ? m_loader->getMeshCount() : 0; }
    uint32_t getMaterialCount() const { return m_loader ? m_loader->getMaterialCount() : 0; }
    glm::vec3 getCameraPosition() const { return m_camera ? m_camera->getPosition() : glm::vec3(0.0f); }
    
    ViewerSettings& getSettings() { return m_settings; }
    const ViewerSettings& getSettings() const { return m_settings; }
    
    // Camera controls for GUI
    void resetCamera();
    
    // Gizmo controls for GUI  
    GizmoMode getGizmoMode() const { return m_gizmo ? m_gizmo->getMode() : GizmoMode::TRANSLATE; }
    void setGizmoMode(GizmoMode mode) { if (m_gizmo) m_gizmo->setMode(mode); }
    OrbitCamera& getCamera() { return *m_camera; }
    GLTFLoader& getLoader() { return *m_loader; }
    
    // Vulkan resources access
    VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }
    
private:
    void createRenderPipelines();
    void createUniformBuffer();
    void updateUniformBuffers();
    void updateTextureDescriptors();
    void renderModel();
    void renderGizmo();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    
    std::unique_ptr<GLTFLoader> m_loader;
    std::unique_ptr<OrbitCamera> m_camera;
    std::unique_ptr<Gizmo> m_gizmo;
    
    ViewerSettings m_settings;
    
    // Vulkan resources
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_solidPipeline{VK_NULL_HANDLE};
    VkPipeline m_wireframePipeline{VK_NULL_HANDLE};
    
    // Uniform buffers
    VkBuffer m_uniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_uniformMemory{VK_NULL_HANDLE};
    void* m_uniformMapped{nullptr};
    
    VkDescriptorSetLayout m_descriptorLayout{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
    
    // Mouse interaction state
    bool m_leftMousePressed{false};
    bool m_rightMousePressed{false};
    double m_lastMouseX{0.0};
    double m_lastMouseY{0.0};
    
    // Model state
    bool m_modelLoaded{false};
    std::string m_modelPath;
    glm::mat4 m_modelMatrix{1.0f};
    glm::vec3 m_modelCenter{0.0f};
    float m_modelRadius{1.0f};
};