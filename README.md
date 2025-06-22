# Vulkan Graphics Application

A modern C++ application demonstrating Vulkan graphics programming concepts.

## Overview

This project implements a Vulkan-based graphics application with a focus on modern C++ practices and clean architecture. It provides a foundation for building more complex graphics applications using Vulkan.

## Features

- Modern C++20 implementation
- GLFW window management
- Vulkan instance and device management
- Validation layers support for debugging
- Cross-platform compatibility (with special handling for macOS/MoltenVK)
- Clean architecture with separated device management

## Prerequisites

- CMake 3.31 or higher
- C++20 compatible compiler
- Vulkan SDK
- GLFW3
- GLM (OpenGL Mathematics)

### macOS Specific Requirements

```bash
# Install dependencies using Homebrew
brew install cmake
brew install vulkan-sdk
brew install glfw
brew install glm
```

## Building the Project

1. Clone the repository
2. Create a build directory:
```bash
mkdir build && cd build
```

3. Generate build files:
```bash
cmake ..
```

4. Build the project:
```bash
cmake --build .
```

## Project Structure

```
├── include/              # Header files
│   ├── VulkanApp.h      # Main application class
│   ├── VulkanDebug.h    # Debug utilities
│   └── VulkanDevice.h   # Device management
├── src/                 # Source files
│   ├── VulkanApp.cpp
│   ├── VulkanDebug.cpp
│   └── VulkanDevice.cpp
├── shaders/             # Shader files
├── assets/             # Asset files
└── CMakeLists.txt      # Build configuration
```

## Architecture

The project is organized into several key components:

- **VulkanApp**: Main application class handling initialization and main loop
- **VulkanDevice**: Handles physical and logical device management
- **VulkanDebug**: Provides debug messenger setup and validation layer support

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
