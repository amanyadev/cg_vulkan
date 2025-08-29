cmake_minimum_required(VERSION 3.20)
project(cg_vulkan)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Fetch ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp  
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PRIVATE
    Vulkan::Vulkan
    glfw
)

# Define source files by component
set(CORE_SOURCES
        src/core/VulkanApp.cpp
        src/core/VulkanDevice.cpp
        src/core/WindowManager.cpp
        src/core/VulkanSync.cpp)

set(DEBUG_SOURCES
        src/debug/VulkanDebug.cpp)

set(RENDERING_SOURCES
        src/rendering/CommandBuffer.cpp
        src/rendering/Framebuffer.cpp
        src/rendering/GraphicsPipeline.cpp
        src/rendering/RenderPass.cpp
        src/rendering/SwapChain.cpp
        src/rendering/UniformBuffer.cpp)

set(UI_SOURCES
        src/ui/DebugUI.cpp)

# Combine all sources
set(SOURCES
        ${CORE_SOURCES}
        ${DEBUG_SOURCES}
        ${RENDERING_SOURCES}
        ${UI_SOURCES}
        main.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add shader compilation
find_program(GLSL_VALIDATOR glslangValidator HINTS
        /usr/local/bin
        /usr/bin
        $ENV{VULKAN_SDK}/bin)

if (NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif ()

# Create shader build targets
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

set(SHADER_SOURCES
        ${SHADER_SOURCE_DIR}/shader.vert
        ${SHADER_SOURCE_DIR}/shader.frag)

foreach (SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    add_custom_command(
            OUTPUT ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv
            COMMAND ${GLSL_VALIDATOR} -V ${SHADER} -o ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv
            DEPENDS ${SHADER}
            COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SHADER_BINARIES ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)
endforeach ()

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES})
add_dependencies(${PROJECT_NAME} Shaders)

# Configure include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${Vulkan_INCLUDE_DIRS})

if (APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            VK_ENABLE_BETA_EXTENSIONS
            VK_USE_PLATFORM_MACOS_MVK)
endif ()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
        imgui)

# Copy shaders to build directory
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER_BINARIES}
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/")
