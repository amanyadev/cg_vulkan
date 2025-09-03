#include "ui/DebugUI.h"
#include "rendering/RenderPass.h"
#include <stdexcept>
#include <iostream>

DebugUI::DebugUI(VulkanDevice* device, SwapChain* swapChain, VkRenderPass renderPass, GLFWwindow* window)
    : m_device(device), m_swapChain(swapChain), m_renderPass(renderPass), m_window(window) {
    
    createDescriptorPool();
    createCommandBuffers();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_device->getInstance();
    init_info.PhysicalDevice = m_device->getPhysicalDevice();
    init_info.Device = m_device->getDevice();
    init_info.QueueFamily = m_device->findQueueFamilies(m_device->getPhysicalDevice()).graphicsFamily.value();
    init_info.Queue = m_device->getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Allocator = nullptr;
    init_info.MinImageCount = m_swapChain->getImages().size();
    init_info.ImageCount = m_swapChain->getImages().size();
    init_info.CheckVkResultFn = nullptr;
    init_info.RenderPass = m_renderPass;
    
    ImGui_ImplVulkan_Init(&init_info);
    m_initialized = true;
}

DebugUI::~DebugUI() {
    if (m_initialized) {
        vkDeviceWaitIdle(m_device->getDevice());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
    }
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
    }
}

void DebugUI::createDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    
    if (vkCreateDescriptorPool(m_device->getDevice(), &pool_info, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create ImGui descriptor pool!");
    }
}

void DebugUI::createCommandBuffers() {
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = m_device->findQueueFamilies(m_device->getPhysicalDevice());
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    if (vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create ImGui command pool!");
    }
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate ImGui command buffer!");
    }
}

void DebugUI::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::render() {
    ImGui::Render();
}

void DebugUI::renderDrawData(VkCommandBuffer commandBuffer) {
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data && draw_data->CmdListsCount > 0) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    }
}

void DebugUI::renderDebugPanel(PerformanceStats& stats, RenderSettings& settings) {
    if (!settings.showDebugUI) return;
    
    ImGui::Begin("Debug Controls", &settings.showDebugUI);
    
    // Performance metrics
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.1f", stats.fps);
        ImGui::Text("Frame Time: %.3f ms", stats.frameTime * 1000.0f);
        ImGui::Text("CPU Time: %.3f ms", stats.cpuTime * 1000.0f);
        ImGui::Text("GPU Time: %.3f ms", stats.gpuTime * 1000.0f);
        
        ImGui::Separator();
        ImGui::Text("Terrain Performance");
        ImGui::Text("Raymarching Steps: Dynamic");
        ImGui::Text("Terrain Quality: %s", stats.qualityLevel == 0 ? "Low" : (stats.qualityLevel == 1 ? "Medium" : "High"));
        
        // Performance graph would go here
        if (ImGui::Button("Reset Stats")) {
            stats = PerformanceStats{};
        }
    }
    
    // Rendering settings
    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Max Raymarching Steps", &settings.maxSteps, 50, 300);
        ImGui::SliderFloat("Max Raymarching Distance", &settings.maxDistance, 50.0f, 500.0f);
        
        const char* qualityItems[] = { "Low", "Medium", "High" };
        ImGui::Combo("Quality Level", &settings.qualityLevel, qualityItems, IM_ARRAYSIZE(qualityItems));
        
        ImGui::Separator();
        ImGui::Text("Terrain Features");
        ImGui::Checkbox("Enable Water", &settings.enableWater);
        ImGui::SliderFloat("View Distance", &settings.viewDistance, 100.0f, 500.0f);
        ImGui::SliderFloat("Fog Density", &settings.fogDensity, 0.001f, 0.02f);
        
        ImGui::Separator();
        ImGui::Text("Visual Effects");
        ImGui::ColorEdit3("Sky Horizon", &settings.skyHorizon.x);
        ImGui::ColorEdit3("Sky Zenith", &settings.skyZenith.x);
        ImGui::ColorEdit3("Sun Color", &settings.sunColor.x);
        ImGui::SliderFloat("Sun Intensity", &settings.sunIntensity, 1.0f, 5.0f);
        
        ImGui::Separator();
        ImGui::Text("Time of Day");
        ImGui::SliderFloat("Time Speed", &settings.timeSpeed, 0.0f, 1.0f);
        if (ImGui::Button("Reset to Noon")) {
            settings.timeOffset = 0.0f;
        }
        
        ImGui::Separator();
        ImGui::Checkbox("V-Sync", &settings.enableVSync);
    }
    
    // Camera info
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", 0.0f, 0.0f, 0.0f); // Will be filled by app
        ImGui::Text("Rotation: (%.1f, %.1f)", 0.0f, 0.0f); // Will be filled by app
    }
    
    // Controls help
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::Text("WASD - Move Camera");
        ImGui::Text("Q/E - Up/Down");
        ImGui::Text("Mouse - Look Around (when captured)");
        ImGui::Text("ESC - Toggle Mouse Capture");
        ImGui::Text("F - Print FPS to Console");
        ImGui::Text("F1 - Toggle Debug UI");
        ImGui::Text("-/+ - Adjust Raymarching Steps");
    }
    
    ImGui::End();
}

void DebugUI::renderViewerPanel(PerformanceStats& stats, GLTFViewer* viewer, BackgroundSettings& backgroundSettings) {
    if (!viewer) return;
    
    ImGui::Begin("glTF Viewer Controls");
    
    // Performance stats
    if (ImGui::CollapsingHeader("Performance")) {
        ImGui::Text("FPS: %.1f (%.2f ms)", stats.fps, stats.frameTime * 1000.0f);
        ImGui::Text("CPU Time: %.2f ms", stats.cpuTime * 1000.0f);
        ImGui::Text("GPU Time: %.2f ms", stats.gpuTime * 1000.0f);
        if (viewer->isModelLoaded()) {
            ImGui::Text("Vertices: %d", viewer->getVertexCount());
            ImGui::Text("Triangles: %d", viewer->getTriangleCount());
            ImGui::Text("Meshes: %d", viewer->getMeshCount());
            ImGui::Text("Materials: %d", viewer->getMaterialCount());
        }
    }
    
    // Model loading
    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Load glTF Model...")) {
            std::vector<FileDialog::Filter> filters = {
                {"glTF Models", "gltf,glb"},
                {"glTF ASCII", "gltf"},
                {"glTF Binary", "glb"}
            };
            
            std::string filePath = FileDialog::openFile(filters);
            if (!filePath.empty()) {
                viewer->loadModel(filePath);
            }
        }
        
        ImGui::SameLine();
        ImGui::Text("Supported: .gltf, .glb");
        
        // EXR texture loading
        if (ImGui::Button("Load EXR Texture...")) {
            std::vector<FileDialog::Filter> exrFilters = {
                {"EXR Images", "exr"},
                {"High Dynamic Range", "hdr,exr"}
            };
            
            std::string exrPath = FileDialog::openFile(exrFilters);
            if (!exrPath.empty()) {
                // For now, just show that EXR loading is available
                ImGui::OpenPopup("EXR Loading");
            }
        }
        
        ImGui::SameLine();
        ImGui::Text("HDR textures: .exr");
        
        // EXR loading popup
        if (ImGui::BeginPopupModal("EXR Loading", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("EXR loading is currently disabled due to dependency issues.");
            ImGui::Text("EXR files can be referenced in glTF models for proper loading.");
            ImGui::Separator();
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        if (viewer->isModelLoaded()) {
            ImGui::Text("Model: %s", viewer->getModelPath().c_str());
            ImGui::Text("Center: (%.2f, %.2f, %.2f)", 
                viewer->getModelCenter().x, 
                viewer->getModelCenter().y, 
                viewer->getModelCenter().z);
            ImGui::Text("Radius: %.2f", viewer->getModelRadius());
        } else {
            ImGui::Text("No model loaded");
        }
    }
    
    // Rendering options
    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ViewerSettings& settings = viewer->getSettings();
        
        // Render mode
        const char* renderModes[] = {"PBR", "Wireframe", "Points", "Normals", "Albedo", "Metallic", "Roughness", "AO"};
        if (ImGui::Combo("Render Mode", &settings.renderMode, renderModes, 8)) {
            // renderMode is now an int, so no conversion needed
        }
        
        // Material overrides
        ImGui::Checkbox("Use Vertex Colors", &settings.useVertexColors);
        ImGui::Checkbox("Show Textures", &settings.showTextures);
        ImGui::Checkbox("Show Normals", &settings.showNormals);
        
        ImGui::ColorEdit3("Material Color", &settings.materialColor[0]);
        ImGui::SliderFloat("Metallic", &settings.metallicFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &settings.roughnessFactor, 0.0f, 1.0f);
        
        // Debug visualization
        ImGui::Checkbox("Show Wireframe Overlay", &settings.showWireframe);
        ImGui::Checkbox("Show Bounding Box", &settings.showBoundingBox);
    }
    
    // Lighting controls
    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
        ViewerSettings& settings = viewer->getSettings();
        
        // Primary Light
        ImGui::Text("Primary Light");
        ImGui::SliderFloat3("Direction##1", &settings.lightDirection[0], -1.0f, 1.0f);
        ImGui::ColorEdit3("Color##1", &settings.lightColor[0]);
        ImGui::SliderFloat("Intensity##1", &settings.lightIntensity, 0.0f, 10.0f);
        
        ImGui::Separator();
        
        // Secondary Light
        ImGui::Text("Secondary Light");
        ImGui::SliderFloat3("Direction##2", &settings.light2Direction[0], -1.0f, 1.0f);
        ImGui::ColorEdit3("Color##2", &settings.light2Color[0]);
        ImGui::SliderFloat("Intensity##2", &settings.light2Intensity, 0.0f, 10.0f);
        
        ImGui::Separator();
        
        // Ambient & IBL
        ImGui::Text("Ambient & IBL");
        ImGui::ColorEdit3("Ambient Color", &settings.ambientColor[0]);
        ImGui::SliderFloat("Ambient Intensity", &settings.ambientIntensity, 0.0f, 3.0f);
        ImGui::SliderFloat("IBL Intensity", &settings.iblIntensity, 0.0f, 3.0f);
        
        ImGui::Separator();
        
        // Post-Processing
        ImGui::Text("Post-Processing");
        ImGui::SliderFloat("Exposure", &settings.exposure, 0.1f, 5.0f);
        ImGui::SliderFloat("Gamma", &settings.gamma, 1.0f, 3.0f);
        
        ImGui::Separator();
        
        // Material Override
        ImGui::Text("Material Override");
        ImGui::SliderFloat("Global Metallic", &settings.metallicFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Global Roughness", &settings.roughnessFactor, 0.0f, 1.0f);
        
        ImGui::Separator();
        
        // Debug Views
        ImGui::Text("Debug Views");
        const char* renderModeItems[] = { "PBR", "Wireframe", "Points", "Normals", "Albedo", "Metallic", "Roughness", "AO" };
        ImGui::Combo("Debug Mode", &settings.renderMode, renderModeItems, IM_ARRAYSIZE(renderModeItems));
        
        ImGui::Separator();
        
        // Light Presets
        ImGui::Text("Light Presets");
        if (ImGui::Button("Daylight")) {
            settings.lightDirection = glm::vec3(-0.5f, -0.8f, -0.3f);
            settings.lightColor = glm::vec3(1.0f, 0.95f, 0.8f);
            settings.lightIntensity = 3.0f;
            settings.light2Direction = glm::vec3(0.3f, -0.6f, 0.7f);
            settings.light2Color = glm::vec3(0.4f, 0.6f, 1.0f);
            settings.light2Intensity = 1.0f;
            settings.ambientColor = glm::vec3(0.3f, 0.4f, 0.6f);
            settings.ambientIntensity = 0.3f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Golden Hour")) {
            settings.lightDirection = glm::vec3(-0.8f, -0.3f, -0.5f);
            settings.lightColor = glm::vec3(1.0f, 0.7f, 0.3f);
            settings.lightIntensity = 2.5f;
            settings.light2Direction = glm::vec3(0.5f, -0.2f, 0.8f);
            settings.light2Color = glm::vec3(0.2f, 0.3f, 0.8f);
            settings.light2Intensity = 0.5f;
            settings.ambientColor = glm::vec3(0.4f, 0.3f, 0.2f);
            settings.ambientIntensity = 0.2f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Studio")) {
            settings.lightDirection = glm::vec3(-0.3f, -0.7f, -0.6f);
            settings.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
            settings.lightIntensity = 4.0f;
            settings.light2Direction = glm::vec3(0.8f, -0.2f, 0.5f);
            settings.light2Color = glm::vec3(0.8f, 0.9f, 1.0f);
            settings.light2Intensity = 2.0f;
            settings.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
            settings.ambientIntensity = 0.1f;
        }
        
        if (ImGui::Button("Night")) {
            settings.lightDirection = glm::vec3(-0.2f, -0.9f, -0.4f);
            settings.lightColor = glm::vec3(0.8f, 0.9f, 1.0f);
            settings.lightIntensity = 0.5f;
            settings.light2Direction = glm::vec3(0.7f, -0.1f, 0.7f);
            settings.light2Color = glm::vec3(0.3f, 0.4f, 0.8f);
            settings.light2Intensity = 0.3f;
            settings.ambientColor = glm::vec3(0.05f, 0.1f, 0.2f);
            settings.ambientIntensity = 0.1f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Dramatic")) {
            settings.lightDirection = glm::vec3(-0.9f, -0.3f, -0.3f);
            settings.lightColor = glm::vec3(1.0f, 0.8f, 0.6f);
            settings.lightIntensity = 6.0f;
            settings.light2Direction = glm::vec3(0.2f, -0.8f, 0.6f);
            settings.light2Color = glm::vec3(0.2f, 0.4f, 1.0f);
            settings.light2Intensity = 1.5f;
            settings.ambientColor = glm::vec3(0.0f, 0.0f, 0.0f);
            settings.ambientIntensity = 0.05f;
        }
    }
    
    // Background settings
    if (ImGui::CollapsingHeader("Background", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* backgroundTypes[] = {"Solid Color", "Gradient", "Skybox"};
        int currentType = static_cast<int>(backgroundSettings.type);
        if (ImGui::Combo("Background Type", &currentType, backgroundTypes, 3)) {
            backgroundSettings.type = static_cast<BackgroundSettings::Type>(currentType);
        }
        
        switch (backgroundSettings.type) {
            case BackgroundSettings::Type::SOLID_COLOR:
                ImGui::ColorEdit3("Background Color", &backgroundSettings.solidColor[0]);
                break;
                
            case BackgroundSettings::Type::GRADIENT:
                ImGui::ColorEdit3("Top Color", &backgroundSettings.gradientTop[0]);
                ImGui::ColorEdit3("Bottom Color", &backgroundSettings.gradientBottom[0]);
                break;
                
            case BackgroundSettings::Type::SKYBOX:
                ImGui::SliderFloat("Environment Rotation", &backgroundSettings.environmentRotation, 0.0f, 360.0f);
                ImGui::SliderFloat("Environment Intensity", &backgroundSettings.environmentIntensity, 0.0f, 3.0f);
                if (ImGui::Button("Load HDR Environment...")) {
                    std::vector<FileDialog::Filter> filters = {
                        {"HDR Images", "hdr,exr"},
                        {"Radiance HDR", "hdr"},
                        {"OpenEXR", "exr"}
                    };
                    
                    std::string filePath = FileDialog::openFile(filters);
                    if (!filePath.empty()) {
                        // TODO: Implement HDR environment loading
                        ImGui::OpenPopup("HDR Loading");
                    }
                }
                if (ImGui::BeginPopup("HDR Loading")) {
                    ImGui::Text("HDR environment loading will be implemented");
                    ImGui::Text("in a future update.");
                    if (ImGui::Button("OK")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                break;
        }
    }
    
    // Camera controls
    if (ImGui::CollapsingHeader("Camera")) {
        ViewerSettings& settings = viewer->getSettings();
        
        if (ImGui::Button("Reset Camera")) {
            viewer->resetCamera();
        }
        
        ImGui::Checkbox("Auto Rotate", &settings.enableAutoRotate);
        if (settings.enableAutoRotate) {
            ImGui::SliderFloat("Rotation Speed", &settings.autoRotateSpeed, 0.1f, 2.0f);
        }
        
        ImGui::Separator();
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", 
            viewer->getCameraPosition().x,
            viewer->getCameraPosition().y, 
            viewer->getCameraPosition().z);
    }
    
    // Gizmo controls
    if (ImGui::CollapsingHeader("Gizmos")) {
        ViewerSettings& settings = viewer->getSettings();
        
        ImGui::Checkbox("Show Gizmos", &settings.showGizmo);
        
        if (settings.showGizmo) {
            const char* gizmoModes[] = {"Translate", "Rotate", "Scale"};
            int currentGizmoMode = static_cast<int>(viewer->getGizmoMode());
            if (ImGui::Combo("Gizmo Mode", &currentGizmoMode, gizmoModes, 3)) {
                viewer->setGizmoMode(static_cast<GizmoMode>(currentGizmoMode));
            }
        }
    }
    
    // Export/Screenshot
    if (ImGui::CollapsingHeader("Export")) {
        if (ImGui::Button("Take Screenshot...")) {
            std::vector<FileDialog::Filter> filters = {
                {"PNG Image", "png"},
                {"JPEG Image", "jpg,jpeg"},
                {"Bitmap Image", "bmp"}
            };
            
            std::string filePath = FileDialog::saveFile(filters);
            if (!filePath.empty()) {
                viewer->takeScreenshot(filePath);
            }
        }
        
        ImGui::Text("Save screenshot as PNG, JPG, or BMP");
    }
    
    // Controls help
    if (ImGui::CollapsingHeader("Controls")) {
        ImGui::Text("Mouse Controls:");
        ImGui::BulletText("Left drag - Orbit camera");
        ImGui::BulletText("Scroll wheel - Zoom");
        ImGui::BulletText("Right drag - Pan camera");
        
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::BulletText("R - Reset camera");
        ImGui::BulletText("1 - Solid rendering mode");
        ImGui::BulletText("2 - Wireframe mode");  
        ImGui::BulletText("G - Toggle gizmos");
        ImGui::BulletText("A - Toggle auto-rotate");
    }
    
    ImGui::End();
}