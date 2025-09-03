#pragma once

#include "core/VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>
#include <string>
#include <vector>
#include <memory>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;
    
    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};

struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    int32_t materialIndex = -1;
    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory indexBufferMemory{VK_NULL_HANDLE};
};

struct Mesh {
    std::vector<Primitive> primitives;
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vertexBufferMemory{VK_NULL_HANDLE};
    uint32_t vertexCount = 0;
};

struct Material {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);
    
    // Texture indices (-1 if not used)
    int baseColorTextureIndex = -1;
    int normalTextureIndex = -1;
    int metallicRoughnessTextureIndex = -1;
    int emissiveTextureIndex = -1;
};

struct Node {
    std::vector<uint32_t> children;
    glm::mat4 matrix = glm::mat4(1.0f);
    int meshIndex = -1;
    
    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    
    glm::mat4 getLocalMatrix() const;
    glm::mat4 getWorldMatrix(const std::vector<Node>& nodes) const;
};

struct Texture {
    VkImage image{VK_NULL_HANDLE};
    VkDeviceMemory imageMemory{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VkSampler sampler{VK_NULL_HANDLE};
    uint32_t width, height;
    uint32_t mipLevels;
    
    // Temporary staging buffer for texture data
    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VkDeviceMemory stagingMemory{VK_NULL_HANDLE};
};

class GLTFLoader {
public:
    GLTFLoader(VulkanDevice* device);
    ~GLTFLoader();
    
    bool loadFromFile(const std::string& filePath);
    void cleanup();
    
    // Rendering
    void render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
    
    // Getters
    const std::vector<Mesh>& getMeshes() const { return m_meshes; }
    const std::vector<Material>& getMaterials() const { return m_materials; }
    const std::vector<Node>& getNodes() const { return m_nodes; }
    const std::vector<Texture>& getTextures() const { return m_textures; }
    
    // Model bounds
    glm::vec3 getCenter() const { return m_center; }
    float getRadius() const { return m_radius; }
    glm::vec3 getMin() const { return m_min; }
    glm::vec3 getMax() const { return m_max; }
    
    // Statistics
    uint32_t getVertexCount() const { return m_totalVertices; }
    uint32_t getTriangleCount() const { return m_totalIndices / 3; }
    uint32_t getMeshCount() const { return static_cast<uint32_t>(m_meshes.size()); }
    uint32_t getMaterialCount() const { return static_cast<uint32_t>(m_materials.size()); }
    
    // Access vertex data for rendering
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }
    VkBuffer getVertexBuffer() const { return m_vertexBuffer; }
    VkBuffer getIndexBuffer() const { return m_indexBuffer; }
    
    // Access default textures
    const Texture& getDefaultAlbedoTexture() const { return m_defaultAlbedoTexture; }
    const Texture& getDefaultNormalTexture() const { return m_defaultNormalTexture; }
    const Texture& getDefaultMetallicRoughnessTexture() const { return m_defaultMetallicRoughnessTexture; }
    const Texture& getDefaultEmissiveTexture() const { return m_defaultEmissiveTexture; }
    const Texture& getDefaultAOTexture() const { return m_defaultAOTexture; }
    
private:
    void loadNode(const tinygltf::Model& model, const tinygltf::Node& node, uint32_t nodeIndex);
    void loadMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh);
    void loadMaterial(const tinygltf::Model& model, const tinygltf::Material& material);
    void loadTexture(const tinygltf::Model& model, const tinygltf::Texture& texture);
    void loadImage(const tinygltf::Model& model, const tinygltf::Image& image, Texture& texture);
    
    void createBuffers();
    void calculateBounds();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createVulkanTexture(Texture& texture, const unsigned char* data, uint32_t width, uint32_t height, int channels);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createDefaultTextures();
    void createCommandPool();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    VulkanDevice* m_device;
    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    
    // glTF data
    tinygltf::Model m_model;
    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<Node> m_nodes;
    std::vector<Texture> m_textures;
    
    // Model bounds
    glm::vec3 m_center{0.0f};
    float m_radius{1.0f};
    glm::vec3 m_min{0.0f};
    glm::vec3 m_max{0.0f};
    
    // Statistics
    uint32_t m_totalVertices{0};
    uint32_t m_totalIndices{0};
    
    // Vertex data for rendering
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    // Vulkan buffers for rendering
    VkBuffer m_vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_vertexBufferMemory{VK_NULL_HANDLE};
    VkBuffer m_indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_indexBufferMemory{VK_NULL_HANDLE};
    
    // Default textures for missing maps
    Texture m_defaultAlbedoTexture{};
    Texture m_defaultNormalTexture{};
    Texture m_defaultMetallicRoughnessTexture{};
    Texture m_defaultEmissiveTexture{};
    Texture m_defaultAOTexture{};
    
    bool m_loaded{false};
};