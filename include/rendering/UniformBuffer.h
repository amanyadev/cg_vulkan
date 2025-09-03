#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>

// glTF viewer uniform buffer (matches shader layout exactly)
struct UniformBufferObject {
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::mat4 normalMatrix;
    
    glm::vec3 cameraPos;
    float time;
    
    // Primary light
    glm::vec3 lightDirection;
    float lightIntensity;
    glm::vec3 lightColor;
    float padding1;
    
    // Secondary light
    glm::vec3 light2Direction;
    float light2Intensity;
    glm::vec3 light2Color;
    float padding2;
    
    // Ambient lighting
    glm::vec3 ambientColor;
    float ambientIntensity;
    
    // IBL and environment
    float exposure;
    float gamma;
    float iblIntensity;
    float shadowIntensity;
    
    // Material override
    float metallicFactor;
    float roughnessFactor;
    int renderMode; // 0=PBR, 1=wireframe, 2=points, 3=normals, etc.
    float padding3;
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