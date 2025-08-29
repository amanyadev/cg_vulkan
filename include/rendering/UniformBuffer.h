#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

struct UniformBufferObject {
    glm::vec3 cameraPos;
    float time;
    glm::vec3 cameraTarget; 
    float aspectRatio;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    
    // Performance and debug settings
    int maxSteps;           // Raymarching max steps
    float maxDistance;      // Raymarching max distance
    int enableTrees;        // 1 = enabled, 0 = disabled
    int enableWater;        // 1 = enabled, 0 = disabled
    int enableClouds;       // 1 = enabled, 0 = disabled
    int qualityLevel;       // 0 = low, 1 = medium, 2 = high
    float treeDistance;     // Max distance to render trees
    float padding;          // Alignment padding
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