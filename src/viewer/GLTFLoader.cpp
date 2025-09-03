#include "viewer/GLTFLoader.h"
#include <iostream>
#include <algorithm>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include "utils/EXRLoader.h"

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    // Normal
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    // Texture coordinates
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    // Color
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

glm::mat4 Node::getLocalMatrix() const {
    return matrix * glm::translate(glm::mat4(1.0f), translation) * 
           glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
}

glm::mat4 Node::getWorldMatrix(const std::vector<Node>& nodes) const {
    // For now, just return local matrix (we'd need parent tracking for full world matrix)
    return getLocalMatrix();
}

GLTFLoader::GLTFLoader(VulkanDevice* device) : m_device(device) {
    createCommandPool();
    createDefaultTextures();
}

GLTFLoader::~GLTFLoader() {
    cleanup();
}

bool GLTFLoader::loadFromFile(const std::string& filePath) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    
    std::cout << "Loading glTF model: " << filePath << std::endl;
    
    bool success = false;
    if (filePath.substr(filePath.find_last_of('.') + 1) == "gltf") {
        success = loader.LoadASCIIFromFile(&m_model, &err, &warn, filePath);
    } else {
        success = loader.LoadBinaryFromFile(&m_model, &err, &warn, filePath);
    }
    
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
        return false;
    }
    
    if (!success) {
        std::cerr << "Failed to load glTF model" << std::endl;
        return false;
    }
    
    std::cout << "Successfully loaded glTF model with:" << std::endl;
    std::cout << "  - " << m_model.meshes.size() << " meshes" << std::endl;
    std::cout << "  - " << m_model.materials.size() << " materials" << std::endl;
    std::cout << "  - " << m_model.textures.size() << " textures" << std::endl;
    std::cout << "  - " << m_model.nodes.size() << " nodes" << std::endl;
    
    // Process the loaded model
    m_meshes.clear();
    m_materials.clear();
    m_nodes.clear();
    m_textures.clear();
    
    // Load materials
    for (const auto& material : m_model.materials) {
        loadMaterial(m_model, material);
    }
    
    // Load textures
    for (const auto& texture : m_model.textures) {
        loadTexture(m_model, texture);
    }
    
    // Load meshes
    for (const auto& mesh : m_model.meshes) {
        loadMesh(m_model, mesh);
    }
    
    // Load nodes
    for (size_t i = 0; i < m_model.nodes.size(); ++i) {
        loadNode(m_model, m_model.nodes[i], static_cast<uint32_t>(i));
    }
    
    // Create Vulkan buffers
    createBuffers();
    calculateBounds();
    
    m_loaded = true;
    return true;
}

void GLTFLoader::loadMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
    Mesh newMesh{};
    
    for (const auto& primitive : mesh.primitives) {
        Primitive newPrimitive{};
        newPrimitive.firstIndex = static_cast<uint32_t>(m_indices.size());
        
        // Load vertex data
        const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
        const auto& posBuffer = model.buffers[posBufferView.buffer];
        const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
        
        // Get other attributes
        const float* normals = nullptr;
        const float* texCoords = nullptr;
        const float* colors = nullptr;
        
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const auto& normalAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
            const auto& normalBufferView = model.bufferViews[normalAccessor.bufferView];
            const auto& normalBuffer = model.buffers[normalBufferView.buffer];
            normals = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);
        }
        
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const auto& texAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
            const auto& texBuffer = model.buffers[texBufferView.buffer];
            texCoords = reinterpret_cast<const float*>(&texBuffer.data[texBufferView.byteOffset + texAccessor.byteOffset]);
        }
        
        if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
            const auto& colorAccessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
            const auto& colorBufferView = model.bufferViews[colorAccessor.bufferView];
            const auto& colorBuffer = model.buffers[colorBufferView.buffer];
            colors = reinterpret_cast<const float*>(&colorBuffer.data[colorBufferView.byteOffset + colorAccessor.byteOffset]);
        }
        
        // Create vertices and track their starting index
        uint32_t vertexStart = static_cast<uint32_t>(m_vertices.size());
        
        for (size_t v = 0; v < posAccessor.count; ++v) {
            Vertex vertex{};
            vertex.position = glm::vec3(positions[v * 3 + 0], positions[v * 3 + 1], positions[v * 3 + 2]);
            
            if (normals) {
                vertex.normal = glm::vec3(normals[v * 3 + 0], normals[v * 3 + 1], normals[v * 3 + 2]);
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            
            if (texCoords) {
                vertex.texCoord = glm::vec2(texCoords[v * 2 + 0], texCoords[v * 2 + 1]);
            }
            
            if (colors) {
                vertex.color = glm::vec4(colors[v * 4 + 0], colors[v * 4 + 1], colors[v * 4 + 2], colors[v * 4 + 3]);
            } else {
                vertex.color = glm::vec4(1.0f);
            }
            
            m_vertices.push_back(vertex);
        }
        
        // Load indices if they exist
        if (primitive.indices >= 0) {
            const auto& indexAccessor = model.accessors[primitive.indices];
            const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = model.buffers[indexBufferView.buffer];
            
            newPrimitive.indexCount = static_cast<uint32_t>(indexAccessor.count);
            
            // Handle different index types
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const uint16_t* indices = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; ++i) {
                    m_indices.push_back(vertexStart + static_cast<uint32_t>(indices[i]));
                }
            } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const uint32_t* indices = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; ++i) {
                    m_indices.push_back(vertexStart + indices[i]);
                }
            } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                const uint8_t* indices = reinterpret_cast<const uint8_t*>(&indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; ++i) {
                    m_indices.push_back(vertexStart + static_cast<uint32_t>(indices[i]));
                }
            }
        } else {
            // Generate indices for non-indexed geometry
            newPrimitive.indexCount = static_cast<uint32_t>(posAccessor.count);
            for (size_t i = 0; i < posAccessor.count; ++i) {
                m_indices.push_back(vertexStart + static_cast<uint32_t>(i));
            }
        }
        
        newPrimitive.materialIndex = primitive.material;
        newMesh.primitives.push_back(newPrimitive);
    }
    
    newMesh.vertexCount = static_cast<uint32_t>(m_vertices.size() - m_totalVertices);
    m_totalVertices = static_cast<uint32_t>(m_vertices.size());
    m_totalIndices = static_cast<uint32_t>(m_indices.size());
    
    m_meshes.push_back(newMesh);
}

void GLTFLoader::loadMaterial(const tinygltf::Model& model, const tinygltf::Material& material) {
    Material newMaterial{};
    
    // Base color
    if (material.values.find("baseColorFactor") != material.values.end()) {
        const auto& factor = material.values.find("baseColorFactor")->second;
        newMaterial.baseColorFactor = glm::vec4(
            factor.ColorFactor()[0],
            factor.ColorFactor()[1], 
            factor.ColorFactor()[2],
            factor.ColorFactor()[3]
        );
    }
    
    // Metallic/Roughness
    if (material.values.find("metallicFactor") != material.values.end()) {
        newMaterial.metallicFactor = static_cast<float>(material.values.find("metallicFactor")->second.Factor());
    }
    
    if (material.values.find("roughnessFactor") != material.values.end()) {
        newMaterial.roughnessFactor = static_cast<float>(material.values.find("roughnessFactor")->second.Factor());
    }
    
    // Emissive
    if (material.additionalValues.find("emissiveFactor") != material.additionalValues.end()) {
        const auto& factor = material.additionalValues.find("emissiveFactor")->second;
        newMaterial.emissiveFactor = glm::vec3(
            factor.ColorFactor()[0],
            factor.ColorFactor()[1],
            factor.ColorFactor()[2]
        );
    }
    
    // Texture indices
    // Base color texture
    if (material.values.find("baseColorTexture") != material.values.end()) {
        newMaterial.baseColorTextureIndex = material.values.find("baseColorTexture")->second.TextureIndex();
    }
    
    // Normal texture
    if (material.additionalValues.find("normalTexture") != material.additionalValues.end()) {
        newMaterial.normalTextureIndex = material.additionalValues.find("normalTexture")->second.TextureIndex();
    }
    
    // Metallic/Roughness texture
    if (material.values.find("metallicRoughnessTexture") != material.values.end()) {
        newMaterial.metallicRoughnessTextureIndex = material.values.find("metallicRoughnessTexture")->second.TextureIndex();
    }
    
    // Emissive texture
    if (material.additionalValues.find("emissiveTexture") != material.additionalValues.end()) {
        newMaterial.emissiveTextureIndex = material.additionalValues.find("emissiveTexture")->second.TextureIndex();
    }
    
    m_materials.push_back(newMaterial);
    
    std::cout << "Loaded material with texture indices: "
              << "base=" << newMaterial.baseColorTextureIndex
              << ", normal=" << newMaterial.normalTextureIndex  
              << ", metallic=" << newMaterial.metallicRoughnessTextureIndex
              << ", emissive=" << newMaterial.emissiveTextureIndex << std::endl;
}

void GLTFLoader::loadTexture(const tinygltf::Model& model, const tinygltf::Texture& texture) {
    Texture newTexture{};
    
    if (texture.source >= 0 && texture.source < static_cast<int>(model.images.size())) {
        const tinygltf::Image& image = model.images[texture.source];
        loadImage(model, image, newTexture);
    }
    
    m_textures.push_back(newTexture);
}

void GLTFLoader::loadImage(const tinygltf::Model& model, const tinygltf::Image& image, Texture& texture) {
    // Check if this is an EXR file
    if (EXRLoader::isEXRFile(image.uri)) {
        HDRImage hdrImage;
        if (hdrImage.loadFromFile(image.uri)) {
            std::cout << "Loaded EXR texture: " << image.uri << std::endl;
            std::cout << "  Dimensions: " << hdrImage.width << "x" << hdrImage.height << std::endl;
            std::cout << "  Channels: " << hdrImage.channels << std::endl;
            
            texture.width = hdrImage.width;
            texture.height = hdrImage.height;
            texture.mipLevels = 1;
            
            // For now, convert to LDR for display
            // TODO: Implement proper HDR texture support in Vulkan pipeline
            auto ldrData = hdrImage.tonemapToLDR();
            
            // Create Vulkan texture from LDR data
            createVulkanTexture(texture, ldrData.data(), texture.width, texture.height, 4);
        } else {
            std::cerr << "Failed to load EXR texture: " << image.uri << std::endl;
        }
    } else {
        // Handle regular images using tinygltf's built-in loading
        if (!image.image.empty()) {
            texture.width = image.width;
            texture.height = image.height;
            texture.mipLevels = 1;
            
            // Determine number of channels
            int channels = image.component;
            
            // Create Vulkan texture from image data
            createVulkanTexture(texture, image.image.data(), texture.width, texture.height, channels);
            
            std::cout << "Loaded texture: " << image.uri << " (" << texture.width << "x" << texture.height 
                      << ", " << channels << " channels)" << std::endl;
        }
    }
}

void GLTFLoader::loadNode(const tinygltf::Model& model, const tinygltf::Node& node, uint32_t nodeIndex) {
    Node newNode{};
    
    // Transform
    if (node.matrix.size() == 16) {
        newNode.matrix = glm::make_mat4(node.matrix.data());
    } else {
        if (node.translation.size() == 3) {
            newNode.translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        }
        if (node.rotation.size() == 4) {
            newNode.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        }
        if (node.scale.size() == 3) {
            newNode.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        }
    }
    
    // Mesh reference
    if (node.mesh >= 0) {
        newNode.meshIndex = node.mesh;
    }
    
    // Children
    for (size_t child : node.children) {
        newNode.children.push_back(static_cast<uint32_t>(child));
    }
    
    m_nodes.push_back(newNode);
}

void GLTFLoader::createBuffers() {
    std::cout << "Creating buffers for " << m_totalVertices << " vertices" << std::endl;
    
    if (m_vertices.empty()) {
        std::cout << "No vertices to create buffers for" << std::endl;
        return;
    }
    
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * m_vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * m_indices.size();
    
    std::cout << "Vertex buffer size: " << vertexBufferSize << " bytes" << std::endl;
    std::cout << "Index buffer size: " << indexBufferSize << " bytes" << std::endl;
    std::cout << "Total vertices in buffer: " << m_vertices.size() << std::endl;
    std::cout << "Total indices in buffer: " << m_indices.size() << std::endl;
    
    // Create vertex buffer
    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_vertexBuffer, m_vertexBufferMemory);
    
    // Copy vertex data
    void* data;
    vkMapMemory(m_device->getDevice(), m_vertexBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, m_vertices.data(), static_cast<size_t>(vertexBufferSize));
    vkUnmapMemory(m_device->getDevice(), m_vertexBufferMemory);
    
    // Create index buffer
    createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_indexBuffer, m_indexBufferMemory);
    
    // Copy index data
    vkMapMemory(m_device->getDevice(), m_indexBufferMemory, 0, indexBufferSize, 0, &data);
    memcpy(data, m_indices.data(), static_cast<size_t>(indexBufferSize));
    vkUnmapMemory(m_device->getDevice(), m_indexBufferMemory);
    
    std::cout << "Buffers created successfully" << std::endl;
}

void GLTFLoader::calculateBounds() {
    if (m_vertices.empty()) {
        m_center = glm::vec3(0.0f);
        m_radius = 1.0f;
        return;
    }
    
    // Calculate actual bounds from vertex data
    m_min = m_vertices[0].position;
    m_max = m_vertices[0].position;
    
    for (const auto& vertex : m_vertices) {
        m_min = glm::min(m_min, vertex.position);
        m_max = glm::max(m_max, vertex.position);
    }
    
    m_center = (m_min + m_max) * 0.5f;
    m_radius = glm::length(m_max - m_min) * 0.5f;
    
    std::cout << "Model bounds: center=" << m_center.x << "," << m_center.y << "," << m_center.z 
              << " radius=" << m_radius << std::endl;
}

void GLTFLoader::cleanup() {
    // Cleanup command pool first
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
    
    // Cleanup main vertex and index buffers
    if (m_vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getDevice(), m_vertexBuffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_vertexBufferMemory, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }
    
    if (m_indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getDevice(), m_indexBuffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_indexBufferMemory, nullptr);
        m_indexBuffer = VK_NULL_HANDLE;
        m_indexBufferMemory = VK_NULL_HANDLE;
    }
    
    // Cleanup Vulkan resources
    for (auto& mesh : m_meshes) {
        if (mesh.vertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device->getDevice(), mesh.vertexBuffer, nullptr);
            vkFreeMemory(m_device->getDevice(), mesh.vertexBufferMemory, nullptr);
        }
        
        for (auto& primitive : mesh.primitives) {
            if (primitive.indexBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_device->getDevice(), primitive.indexBuffer, nullptr);
                vkFreeMemory(m_device->getDevice(), primitive.indexBufferMemory, nullptr);
            }
        }
    }
    
    for (auto& texture : m_textures) {
        if (texture.image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device->getDevice(), texture.image, nullptr);
            vkFreeMemory(m_device->getDevice(), texture.imageMemory, nullptr);
        }
        if (texture.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device->getDevice(), texture.imageView, nullptr);
        }
        if (texture.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device->getDevice(), texture.sampler, nullptr);
        }
        if (texture.stagingBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device->getDevice(), texture.stagingBuffer, nullptr);
            vkFreeMemory(m_device->getDevice(), texture.stagingMemory, nullptr);
        }
    }
    
    // Cleanup default textures
    auto cleanupTexture = [this](Texture& texture) {
        if (texture.image != VK_NULL_HANDLE) {
            vkDestroyImage(m_device->getDevice(), texture.image, nullptr);
            vkFreeMemory(m_device->getDevice(), texture.imageMemory, nullptr);
        }
        if (texture.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device->getDevice(), texture.imageView, nullptr);
        }
        if (texture.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device->getDevice(), texture.sampler, nullptr);
        }
        if (texture.stagingBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device->getDevice(), texture.stagingBuffer, nullptr);
            vkFreeMemory(m_device->getDevice(), texture.stagingMemory, nullptr);
        }
    };
    
    cleanupTexture(m_defaultAlbedoTexture);
    cleanupTexture(m_defaultNormalTexture);
    cleanupTexture(m_defaultMetallicRoughnessTexture);
    cleanupTexture(m_defaultEmissiveTexture);
    cleanupTexture(m_defaultAOTexture);
    
    m_meshes.clear();
    m_materials.clear();
    m_nodes.clear();
    m_textures.clear();
    m_vertices.clear();
    m_indices.clear();
    m_loaded = false;
}

void GLTFLoader::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    if (!m_loaded || m_vertices.empty()) {
        return;
    }
    
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    // Draw indexed
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void GLTFLoader::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                             VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getDevice(), buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }
    
    vkBindBufferMemory(m_device->getDevice(), buffer, bufferMemory, 0);
}

void GLTFLoader::createVulkanTexture(Texture& texture, const unsigned char* data, uint32_t width, uint32_t height, int channels) {
    texture.width = width;
    texture.height = height;
    texture.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    
    VkDeviceSize imageSize = width * height * 4; // Force RGBA
    
    // Create staging buffer
    createBuffer(imageSize, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                texture.stagingBuffer,
                texture.stagingMemory);
    
    // Copy texture data to staging buffer
    void* mappedData;
    vkMapMemory(m_device->getDevice(), texture.stagingMemory, 0, imageSize, 0, &mappedData);
    
    // Convert to RGBA if necessary
    if (channels == 4) {
        memcpy(mappedData, data, imageSize);
    } else if (channels == 3) {
        // Convert RGB to RGBA
        unsigned char* rgba = (unsigned char*)mappedData;
        for (uint32_t i = 0; i < width * height; ++i) {
            rgba[i*4 + 0] = data[i*3 + 0]; // R
            rgba[i*4 + 1] = data[i*3 + 1]; // G
            rgba[i*4 + 2] = data[i*3 + 2]; // B
            rgba[i*4 + 3] = 255;           // A
        }
    } else if (channels == 1) {
        // Convert grayscale to RGBA
        unsigned char* rgba = (unsigned char*)mappedData;
        for (uint32_t i = 0; i < width * height; ++i) {
            rgba[i*4 + 0] = data[i]; // R
            rgba[i*4 + 1] = data[i]; // G
            rgba[i*4 + 2] = data[i]; // B
            rgba[i*4 + 3] = 255;     // A
        }
    } else {
        // Fill with white for other formats
        memset(mappedData, 255, imageSize);
    }
    
    vkUnmapMemory(m_device->getDevice(), texture.stagingMemory);
    
    // Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = texture.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateImage(m_device->getDevice(), &imageInfo, nullptr, &texture.image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image!");
    }
    
    // Allocate image memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->getDevice(), texture.image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &texture.imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory!");
    }
    
    vkBindImageMemory(m_device->getDevice(), texture.image, texture.imageMemory, 0);
    
    // Transition image layout and copy buffer to image
    transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);
    copyBufferToImage(texture.stagingBuffer, texture.image, width, height);
    
    // Generate mipmaps
    generateMipmaps(texture.image, VK_FORMAT_R8G8B8A8_SRGB, width, height, texture.mipLevels);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = texture.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(m_device->getDevice(), &viewInfo, nullptr, &texture.imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view!");
    }
    
    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE; // Disabled to avoid validation error
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(texture.mipLevels);
    samplerInfo.mipLodBias = 0.0f;
    
    if (vkCreateSampler(m_device->getDevice(), &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }
    
    std::cout << "Created Vulkan texture: " << width << "x" << height 
              << " with " << channels << " channels, " << texture.mipLevels << " mip levels" << std::endl;
}

void GLTFLoader::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    endSingleTimeCommands(commandBuffer);
}

void GLTFLoader::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    endSingleTimeCommands(commandBuffer);
}

void GLTFLoader::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_device->getPhysicalDevice(), imageFormat, &formatProperties);
    
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }
    
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;
    
    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                           0, nullptr, 0, nullptr, 1, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      1, &blit, VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                           0, nullptr, 0, nullptr, 1, &barrier);
        
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                       0, nullptr, 0, nullptr, 1, &barrier);
    
    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer GLTFLoader::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void GLTFLoader::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->getGraphicsQueue());
    
    vkFreeCommandBuffers(m_device->getDevice(), m_commandPool, 1, &commandBuffer);
}

uint32_t GLTFLoader::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

void GLTFLoader::createDefaultTextures() {
    // Create default albedo texture (white)
    {
        unsigned char whitePixel[] = {255, 255, 255, 255};
        createVulkanTexture(m_defaultAlbedoTexture, whitePixel, 1, 1, 4);
    }
    
    // Create default normal texture (neutral normal: 128, 128, 255, 255)
    {
        unsigned char normalPixel[] = {128, 128, 255, 255};
        createVulkanTexture(m_defaultNormalTexture, normalPixel, 1, 1, 4);
    }
    
    // Create default metallic/roughness texture (non-metallic, medium roughness: 0, 128, 0, 255)
    {
        unsigned char metallicRoughnessPixel[] = {0, 128, 0, 255}; // G=roughness, B=metallic
        createVulkanTexture(m_defaultMetallicRoughnessTexture, metallicRoughnessPixel, 1, 1, 4);
    }
    
    // Create default emissive texture (black)
    {
        unsigned char blackPixel[] = {0, 0, 0, 255};
        createVulkanTexture(m_defaultEmissiveTexture, blackPixel, 1, 1, 4);
    }
    
    // Create default AO texture (white - no occlusion)
    {
        unsigned char whitePixel[] = {255, 255, 255, 255};
        createVulkanTexture(m_defaultAOTexture, whitePixel, 1, 1, 4);
    }
    
    std::cout << "Created default PBR textures" << std::endl;
}

void GLTFLoader::createCommandPool() {
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies(m_device->getPhysicalDevice());
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    if (vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}