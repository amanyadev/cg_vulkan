# Vulkan Graphics Engine

A modern C++ Vulkan-based graphics engine implementation.

## Project Structure

```
├── include/                    # Header files
│   ├── core/                  # Core engine components
│   │   ├── VulkanApp.h       # Main application class
│   │   ├── VulkanDevice.h    # Device management
│   │   └── WindowManager.h   # GLFW window handling
│   ├── debug/                 # Debug utilities
│   │   └── VulkanDebug.h    # Validation and debug messaging
│   ├── rendering/            # Rendering components
│   │   ├── CommandBuffer.h  # Command buffer management
│   │   ├── Framebuffer.h   # Framebuffer handling
│   │   ├── GraphicsPipeline.h # Graphics pipeline setup
│   │   ├── RenderPass.h    # Render pass configuration
│   │   └── SwapChain.h     # Swap chain management
│   └── resources/           # Resource management (future)
│
├── src/                     # Implementation files
│   ├── core/               # Core implementations
│   │   ├── VulkanApp.cpp
│   │   ├── VulkanDevice.cpp
│   │   └── WindowManager.cpp
│   ├── debug/              # Debug implementations
│   │   └── VulkanDebug.cpp
│   └── rendering/          # Rendering implementations
│       ├── CommandBuffer.cpp
│       ├── Framebuffer.cpp
│       ├── GraphicsPipeline.cpp
│       ├── RenderPass.cpp
│       └── SwapChain.cpp
│
├── shaders/                # Shader files
│   ├── compile.sh         # Shader compilation script
│   ├── src/              # Shader source files
│   │   ├── shader.vert   # Vertex shader
│   │   └── shader.frag   # Fragment shader
│   └── compiled/         # Compiled SPIR-V shaders
│       ├── shader.vert.spv
│       └── shader.frag.spv
│
├── assets/                # Engine assets
│   ├── textures/         # Texture files
│   ├── models/           # 3D model files
│   └── materials/        # Material definitions
│
├── docs/                  # Documentation
│   ├── architecture.md   # Engine architecture
│   ├── setup.md         # Setup instructions
│   └── api/             # API documentation
│
├── tests/                # Test files
│   ├── unit/            # Unit tests
│   └── integration/     # Integration tests
│
└── CMakeLists.txt       # Build configuration
```

## Components

### Core (core/)
1. **VulkanApp**: Main application class that orchestrates all Vulkan components
2. **VulkanDevice**: Physical and logical device management
3. **WindowManager**: GLFW window creation and management

### Debug (debug/)
- **VulkanDebug**: Validation layers and debug messaging utilities

### Rendering (rendering/)
1. **SwapChain**: Swap chain creation and management
2. **RenderPass**: Render pass configuration with:
   - Color attachment setup
   - Subpass management
   - Dependencies handling
3. **GraphicsPipeline**: Graphics pipeline setup with:
   - Shader stage configuration
   - Fixed-function state setup
   - Pipeline layout management
4. **Framebuffer**: Framebuffer creation and management
5. **CommandBuffer**: Command buffer handling with:
   - Command pool management
   - Command recording
   - Submission management

## Build Requirements

- CMake 3.20+
- Vulkan SDK
- GLFW3
- C++17 compiler

## Features

- [x] Vulkan instance creation
- [x] Physical device selection
- [x] Logical device creation
- [x] Swap chain setup
- [x] Graphics pipeline creation
- [x] Shader compilation
- [x] Command buffer management
- [x] Basic triangle rendering setup
- [ ] Vertex buffer support
- [ ] Uniform buffers
- [ ] Texture mapping
- [ ] Multiple render passes

## Building

1. Ensure Vulkan SDK is installed
2. Configure with CMake:
```bash
mkdir build && cd build
cmake ..
```
3. Build:
```bash
cmake --build .
```

## Shader Compilation
Shaders are automatically compiled during the build process. Source GLSL shaders are located in the `shaders/` directory:
- `shader.vert`: Vertex shader
- `shader.frag`: Fragment shader

## Architecture

The engine follows a modular design where each Vulkan concept is encapsulated in its own class:

1. **Initialization Flow**:
   ```
   VulkanApp
   ├── WindowManager
   ├── VulkanDevice
   ├── SwapChain
   └── GraphicsPipeline
       ├── RenderPass
       ├── Framebuffer
       └── CommandBuffer
   ```

2. **Render Flow**:
   ```
   Render Loop
   ├── Acquire next image
   ├── Record command buffer
   ├── Submit to queue
   └── Present
   ```

## Future Improvements

1. Memory management system
2. Multiple render passes
3. Depth and stencil support
4. Resource management
5. Scene graph
6. Material system

## License

This project is open-source and available under the MIT License.
