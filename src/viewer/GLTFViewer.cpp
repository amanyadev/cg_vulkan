#include "viewer/GLTFViewer.h"
#include "rendering/UniformBuffer.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

GLTFViewer::GLTFViewer(VulkanDevice* device, SwapChain* swapChain) 
    : m_device(device), m_swapChain(swapChain) {
}

GLTFViewer::~GLTFViewer() {
    cleanup();
}

void GLTFViewer::initialize() {
    std::cout << "Initializing glTF Viewer..." << std::endl;
    
    // Create components
    m_loader = std::make_unique<GLTFLoader>(m_device);
    m_camera = std::make_unique<OrbitCamera>();
    // m_gizmo will be created later
    
    // Initialize camera with default settings
    m_camera->setFOV(45.0f);
    m_camera->setNearFar(0.1f, 1000.0f);
    m_camera->setDistanceLimits(0.1f, 100.0f);
    m_camera->setPitchLimits(-89.0f, 89.0f);
    m_camera->setSmoothing(10.0f); // Smooth camera movement
    m_camera->lookAt(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f);
    
    // Create render pipelines
    createRenderPipelines();
    
    std::cout << "glTF Viewer initialized successfully" << std::endl;
}

void GLTFViewer::loadModel(const std::string& filePath) {
    std::cout << "Loading model: " << filePath << std::endl;
    
    if (m_loader->loadFromFile(filePath)) {
        m_modelLoaded = true;
        m_modelPath = filePath;
        m_modelCenter = m_loader->getCenter();
        m_modelRadius = m_loader->getRadius();
        
        // Adjust camera to fit the model
        m_camera->lookAt(m_modelCenter, m_modelRadius);
        
        // Update texture descriptors with the loaded model's textures
        updateTextureDescriptors();
        
        std::cout << "Model loaded successfully!" << std::endl;
        std::cout << "  - Vertices: " << m_loader->getVertexCount() << std::endl;
        std::cout << "  - Triangles: " << m_loader->getTriangleCount() << std::endl;
        std::cout << "  - Meshes: " << m_loader->getMeshCount() << std::endl;
        std::cout << "  - Materials: " << m_loader->getMaterialCount() << std::endl;
    } else {
        std::cerr << "Failed to load model: " << filePath << std::endl;
    }
}

void GLTFViewer::update(float deltaTime) {
    // Update camera
    m_camera->update(deltaTime);
    
    // Update auto rotation if enabled
    if (m_settings.enableAutoRotate) {
        m_camera->orbit(m_settings.autoRotateSpeed * deltaTime * 10.0f, 0.0f);
    }
}

void GLTFViewer::render() {
    if (!m_modelLoaded) {
        return; // Nothing to render
    }
    
    // Update uniform buffers
    updateUniformBuffers();
    
    // Render model
    renderModel();
    
    // Render gizmo if enabled
    if (m_settings.showGizmo && m_gizmo) {
        renderGizmo();
    }
}

void GLTFViewer::cleanup() {
    if (m_loader) {
        m_loader->cleanup();
    }
    
    // Cleanup Vulkan resources
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
    
    if (m_solidPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->getDevice(), m_solidPipeline, nullptr);
    }
    
    if (m_wireframePipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->getDevice(), m_wireframePipeline, nullptr);
    }
    
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->getDevice(), m_pipelineLayout, nullptr);
    }
}

// Input handling
void GLTFViewer::onMouseMove(double xpos, double ypos) {
    static bool firstMouse = true;
    if (firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        firstMouse = false;
    }
    
    double deltaX = xpos - m_lastMouseX;
    double deltaY = ypos - m_lastMouseY;
    
    if (m_leftMousePressed) {
        // Left button: Orbit camera around target
        float sensitivity = 0.3f; // Better sensitivity for orbit
        m_camera->orbit(static_cast<float>(deltaX) * sensitivity, static_cast<float>(-deltaY) * sensitivity);
    } else if (m_rightMousePressed) {
        // Right button: Pan camera
        float panSensitivity = 0.01f; // Better sensitivity for panning
        m_camera->pan(static_cast<float>(deltaX) * panSensitivity, static_cast<float>(deltaY) * panSensitivity);
    }
    
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
}

void GLTFViewer::onMouseButton(int button, int action, int mods) {
    bool pressed = (action == 1); // GLFW_PRESS = 1
    
    if (button == 0) { // Left mouse button
        m_leftMousePressed = pressed;
    } else if (button == 1) { // Right mouse button  
        m_rightMousePressed = pressed;
    }
}

void GLTFViewer::onScroll(double xoffset, double yoffset) {
    // Zoom with mouse wheel
    m_camera->zoom(static_cast<float>(-yoffset) * 0.1f);
}

void GLTFViewer::onKey(int key, int scancode, int action, int mods) {
    if (action == 1) { // GLFW_PRESS
        switch (key) {
            case 82: // R key - Reset camera
                resetCamera();
                std::cout << "Camera reset" << std::endl;
                break;
            case 49: // 1 key - PBR mode
                m_settings.renderMode = 0;
                std::cout << "Switched to PBR mode" << std::endl;
                break;
            case 50: // 2 key - Wireframe mode
                m_settings.renderMode = 1;
                std::cout << "Switched to wireframe mode" << std::endl;
                break;
            case 51: // 3 key - Points mode
                m_settings.renderMode = 2;
                std::cout << "Switched to points mode" << std::endl;
                break;
            case 71: // G key - Toggle gizmo
                m_settings.showGizmo = !m_settings.showGizmo;
                std::cout << "Gizmo " << (m_settings.showGizmo ? "enabled" : "disabled") << std::endl;
                break;
            case 65: // A key - Toggle auto rotate
                m_settings.enableAutoRotate = !m_settings.enableAutoRotate;
                m_camera->setAutoRotate(m_settings.enableAutoRotate, m_settings.autoRotateSpeed);
                std::cout << "Auto rotate " << (m_settings.enableAutoRotate ? "enabled" : "disabled") << std::endl;
                break;
            case 87: // W key - Toggle wireframe overlay
                m_settings.showWireframe = !m_settings.showWireframe;
                std::cout << "Wireframe overlay " << (m_settings.showWireframe ? "enabled" : "disabled") << std::endl;
                break;
            case 66: // B key - Toggle bounding box
                m_settings.showBoundingBox = !m_settings.showBoundingBox;
                std::cout << "Bounding box " << (m_settings.showBoundingBox ? "enabled" : "disabled") << std::endl;
                break;
        }
    }
}

void GLTFViewer::takeScreenshot(const std::string& filePath) {
    std::cout << "Taking screenshot: " << filePath << std::endl;
    // Screenshot functionality will be implemented later
}

void GLTFViewer::renderUI() {
    // UI rendering will be integrated with ImGui
}

// Private methods implementation
void GLTFViewer::createRenderPipelines() {
    // Create descriptor set layout
    std::array<VkDescriptorSetLayoutBinding, 6> bindings{};
    
    // Binding 0: Uniform buffer
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;
    
    // Binding 1: Albedo texture
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;
    
    // Binding 2: Normal texture
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = nullptr;
    
    // Binding 3: Metallic/Roughness texture
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[3].pImmutableSamplers = nullptr;
    
    // Binding 4: Emissive texture
    bindings[4].binding = 4;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[4].pImmutableSamplers = nullptr;
    
    // Binding 5: AO texture
    bindings[5].binding = 5;
    bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[5].descriptorCount = 1;
    bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[5].pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(m_device->getDevice(), &layoutInfo, nullptr, &m_descriptorLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    
    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorLayout;
    
    if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    
    // Create uniform buffer
    createUniformBuffer();
    
    std::cout << "Render pipelines created successfully" << std::endl;
}

void GLTFViewer::createUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    // Create uniform buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_device->getDevice(), &bufferInfo, nullptr, &m_uniformBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create uniform buffer!");
    }
    
    // Allocate memory for the buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getDevice(), m_uniformBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &m_uniformMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate uniform buffer memory!");
    }
    
    vkBindBufferMemory(m_device->getDevice(), m_uniformBuffer, m_uniformMemory, 0);
    
    // Create descriptor pool
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 5; // 5 texture samplers
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    
    if (vkCreateDescriptorPool(m_device->getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
    
    // Create descriptor set
    VkDescriptorSetAllocateInfo allocInfo2{};
    allocInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo2.descriptorPool = m_descriptorPool;
    allocInfo2.descriptorSetCount = 1;
    allocInfo2.pSetLayouts = &m_descriptorLayout;
    
    if (vkAllocateDescriptorSets(m_device->getDevice(), &allocInfo2, &m_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set!");
    }
    
    // Update descriptor set - we'll do this when model is loaded to bind actual textures
    VkDescriptorBufferInfo bufferInfo2{};
    bufferInfo2.buffer = m_uniformBuffer;
    bufferInfo2.offset = 0;
    bufferInfo2.range = sizeof(UniformBufferObject);
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo2;
    
    // Initial uniform buffer binding
    vkUpdateDescriptorSets(m_device->getDevice(), 1, &descriptorWrite, 0, nullptr);
    
    // Update texture bindings with default textures initially
    updateTextureDescriptors();
}

void GLTFViewer::updateUniformBuffers() {
    if (!m_uniformBuffer) return;
    
    UniformBufferObject ubo{};
    
    // Model matrix (identity for now)
    ubo.modelMatrix = glm::mat4(1.0f);
    
    // View and projection matrices from camera
    ubo.viewMatrix = m_camera->getViewMatrix();
    float aspectRatio = static_cast<float>(m_swapChain->getExtent().width) / static_cast<float>(m_swapChain->getExtent().height);
    ubo.projMatrix = m_camera->getProjectionMatrix(aspectRatio);
    
    // Normal matrix
    ubo.normalMatrix = glm::transpose(glm::inverse(ubo.modelMatrix));
    
    // Camera position
    ubo.cameraPos = m_camera->getPosition();
    
    // Time (can be used for animations)
    ubo.time = 0.0f; // Will be set by application
    
    // Primary lighting
    ubo.lightDirection = glm::normalize(m_settings.lightDirection);
    ubo.lightIntensity = m_settings.lightIntensity;
    ubo.lightColor = m_settings.lightColor;
    
    // Secondary lighting
    ubo.light2Direction = glm::normalize(m_settings.light2Direction);
    ubo.light2Intensity = m_settings.light2Intensity;
    ubo.light2Color = m_settings.light2Color;
    
    // Ambient lighting
    ubo.ambientColor = m_settings.ambientColor;
    ubo.ambientIntensity = m_settings.ambientIntensity;
    
    // IBL and environment
    ubo.exposure = m_settings.exposure;
    ubo.gamma = m_settings.gamma;
    ubo.iblIntensity = m_settings.iblIntensity;
    ubo.shadowIntensity = m_settings.shadowIntensity;
    
    // Material properties
    ubo.metallicFactor = m_settings.metallicFactor;
    ubo.roughnessFactor = m_settings.roughnessFactor;
    ubo.renderMode = m_settings.renderMode;
    
    // Update buffer
    void* data;
    vkMapMemory(m_device->getDevice(), m_uniformMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device->getDevice(), m_uniformMemory);
}

void GLTFViewer::updateTextureDescriptors() {
    if (!m_loader) return;
    
    // Get model textures and materials
    const auto& textures = m_loader->getTextures();
    const auto& materials = m_loader->getMaterials();
    
    // Select textures to use - try to use from first material if available, otherwise use defaults
    const Texture* albedoTexture = &m_loader->getDefaultAlbedoTexture();
    const Texture* normalTexture = &m_loader->getDefaultNormalTexture();
    const Texture* metallicRoughnessTexture = &m_loader->getDefaultMetallicRoughnessTexture();
    const Texture* emissiveTexture = &m_loader->getDefaultEmissiveTexture();
    const Texture* aoTexture = &m_loader->getDefaultAOTexture();
    
    // If we have materials and textures, use them
    if (!materials.empty() && !textures.empty()) {
        const Material& material = materials[0]; // Use first material
        
        // Use actual model textures if they exist
        if (material.baseColorTextureIndex >= 0 && material.baseColorTextureIndex < static_cast<int>(textures.size())) {
            albedoTexture = &textures[material.baseColorTextureIndex];
        }
        if (material.normalTextureIndex >= 0 && material.normalTextureIndex < static_cast<int>(textures.size())) {
            normalTexture = &textures[material.normalTextureIndex];
        }
        if (material.metallicRoughnessTextureIndex >= 0 && material.metallicRoughnessTextureIndex < static_cast<int>(textures.size())) {
            metallicRoughnessTexture = &textures[material.metallicRoughnessTextureIndex];
        }
        if (material.emissiveTextureIndex >= 0 && material.emissiveTextureIndex < static_cast<int>(textures.size())) {
            emissiveTexture = &textures[material.emissiveTextureIndex];
        }
    }
    
    // Create descriptor image infos
    std::array<VkDescriptorImageInfo, 5> imageInfos{};
    
    // Albedo texture
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[0].imageView = albedoTexture->imageView;
    imageInfos[0].sampler = albedoTexture->sampler;
    
    // Normal texture
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[1].imageView = normalTexture->imageView;
    imageInfos[1].sampler = normalTexture->sampler;
    
    // Metallic/Roughness texture
    imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[2].imageView = metallicRoughnessTexture->imageView;
    imageInfos[2].sampler = metallicRoughnessTexture->sampler;
    
    // Emissive texture
    imageInfos[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[3].imageView = emissiveTexture->imageView;
    imageInfos[3].sampler = emissiveTexture->sampler;
    
    // AO texture
    imageInfos[4].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[4].imageView = aoTexture->imageView;
    imageInfos[4].sampler = aoTexture->sampler;
    
    // Create write descriptor sets
    std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
    
    for (size_t i = 0; i < 5; ++i) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_descriptorSet;
        descriptorWrites[i].dstBinding = static_cast<uint32_t>(i + 1); // Bindings 1-5
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pImageInfo = &imageInfos[i];
    }
    
    vkUpdateDescriptorSets(m_device->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    
    if (!materials.empty() && !textures.empty()) {
        std::cout << "Updated texture descriptors with model textures" << std::endl;
    } else {
        std::cout << "Updated texture descriptors with default textures" << std::endl;
    }
}

void GLTFViewer::renderModel() {
    if (!m_modelLoaded || !m_loader) return;
    
    // For now, just log that we're rendering
    // Full implementation will require command buffer recording
    // which needs to be integrated with the main render loop
    
    // This will be called from VulkanApp's drawFrame() method
    // The actual rendering commands will be recorded in the command buffer
}

void GLTFViewer::renderToCommandBuffer(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    if (!m_modelLoaded || !m_loader) return;
    
    // Let the GLTFLoader render to the command buffer
    m_loader->render(commandBuffer, pipelineLayout);
}

void GLTFViewer::renderGizmo() {
    // Will implement gizmo rendering
}

void GLTFViewer::resetCamera() {
    if (m_camera) {
        if (m_modelLoaded) {
            m_camera->lookAt(m_modelCenter, m_modelRadius);
        } else {
            m_camera->lookAt(glm::vec3(0.0f), 5.0f);
        }
    }
}

uint32_t GLTFViewer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}