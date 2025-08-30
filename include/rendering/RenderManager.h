#pragma once

#include "core/VulkanDevice.h"
#include "rendering/SwapChain.h"
#include "scene/Scene.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

// Simple terrain-only uniform buffer
struct TerrainUniformData {
    glm::vec3 cameraPos;
    float time;
    glm::vec3 cameraTarget;
    float aspectRatio;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    
    // Scene lighting
    glm::vec3 sunDirection;
    float sunIntensity;
    glm::vec3 sunColor;
    float padding1;
    glm::vec3 ambientColor;
    float ambientIntensity;
    glm::vec3 skyColorHorizon;
    float padding2;
    glm::vec3 skyColorZenith;
    float padding3;
    glm::vec3 fogColor;
    float fogDensity;
    
    // Terrain settings
    float terrainScale;
    float terrainHeight;
    float waterLevel;
    int enableWater;
    
    // Quality settings
    int qualityLevel;
    float viewDistance;
    float padding4[2];
};

class RenderManager {
public:
    RenderManager(VulkanDevice* device, SwapChain* swapChain);
    ~RenderManager();
    
    void initialize();
    void render(Scene* scene, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    void cleanup();
    
    // Camera and view settings
    void updateCamera(const glm::vec3& position, const glm::vec3& target);
    void setViewport(uint32_t width, uint32_t height);
    
private:
    void createTerrainPipeline();
    void createEntityPipeline();
    void createUniformBuffers();
    void updateTerrainUniforms(Scene* scene, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    void renderTerrain();
    void renderEntities(Scene* scene);
    
    VulkanDevice* m_device;
    SwapChain* m_swapChain;
    
    // Terrain rendering
    VkPipelineLayout m_terrainPipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_terrainPipeline{VK_NULL_HANDLE};
    VkBuffer m_terrainUniformBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_terrainUniformMemory{VK_NULL_HANDLE};
    void* m_terrainUniformMapped{nullptr};
    VkDescriptorSetLayout m_terrainDescriptorLayout{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    VkDescriptorSet m_terrainDescriptorSet{VK_NULL_HANDLE};
    
    // Camera state
    glm::vec3 m_cameraPos{0.0f, 15.0f, -25.0f};
    glm::vec3 m_cameraTarget{0.0f, 5.0f, 0.0f};
    uint32_t m_viewportWidth{640};
    uint32_t m_viewportHeight{480};
    
    // Timing
    float m_time{0.0f};
    std::chrono::high_resolution_clock::time_point m_startTime;
};