#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

// Simplified terrain-only uniform buffer
struct UniformBufferObject {
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

class UniformBuffer {
public:
    UniformBuffer(VulkanDevice* device, size_t bufferSize);
    ~UniformBuffer();

    void updateBuffer(const UniformBufferObject& ubo);
    VkBuffer getBuffer() const { return m_buffer; }
    VkDeviceMemory getBufferMemory() const { return m_bufferMemory; }
    VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }

private:
    void createBuffer();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VulkanDevice* m_device;
    size_t m_bufferSize;
    
    VkBuffer m_buffer{VK_NULL_HANDLE};
    VkDeviceMemory m_bufferMemory{VK_NULL_HANDLE};
    void* m_mappedMemory{nullptr};
    
    VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
};