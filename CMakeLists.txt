cmake_minimum_required(VERSION 3.31)
project(cg_vulkan)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(cg_vulkan main.cpp
        src/VulkanApp.cpp
        src/VulkanDebug.cpp
        include/VulkanApp.h
        include/VulkanDebug.h)

# include Vulkan headers and link against Vulkan libraries
target_include_directories(cg_vulkan PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(cg_vulkan PRIVATE ${Vulkan_LIBRARIES})

# include GLM headers
find_path(GLM_INCLUDE_DIR glm/glm.hpp PATHS /opt/homebrew/opt/glm/include)
target_include_directories(cg_vulkan PRIVATE ${GLM_INCLUDE_DIR})

# include GLFW headers and link against GLFW libraries
target_link_libraries(cg_vulkan PRIVATE glfw)