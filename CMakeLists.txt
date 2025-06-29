cmake_minimum_required(VERSION 3.31)
project(cg_vulkan)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Add shader compilation
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/local/bin /usr/bin $ENV{VULKAN_SDK}/bin)

function(compile_shader SHADER_NAME)
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders/
            COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/shaders/${SHADER_NAME} -o ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv
            DEPENDS ${CMAKE_SOURCE_DIR}/shaders/${SHADER_NAME}
            COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SHADER_BINARY_FILES ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv)
endfunction()

compile_shader(shader.vert)
compile_shader(shader.frag)

add_custom_target(
        Shaders
        DEPENDS
        ${CMAKE_BINARY_DIR}/shaders/shader.vert.spv
        ${CMAKE_BINARY_DIR}/shaders/shader.frag.spv
)

add_executable(cg_vulkan main.cpp
        src/VulkanApp.cpp
        src/VulkanDebug.cpp
        src/VulkanDevice.cpp
        src/WindowManager.cpp
        src/SwapChain.cpp
        src/GraphicsPipeline.cpp
        src/RenderPass.cpp
        src/Framebuffer.cpp
        include/VulkanApp.h
        include/VulkanDebug.h
        include/VulkanDevice.h
        include/WindowManager.h
        include/SwapChain.h
        include/GraphicsPipeline.h
        include/RenderPass.h
        include/Framebuffer.h)

add_dependencies(cg_vulkan Shaders)

# include Vulkan headers and link against Vulkan libraries
target_include_directories(cg_vulkan PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(cg_vulkan PRIVATE ${Vulkan_LIBRARIES})

# include GLM headers
find_path(GLM_INCLUDE_DIR glm/glm.hpp PATHS /opt/homebrew/opt/glm/include)
target_include_directories(cg_vulkan PRIVATE ${GLM_INCLUDE_DIR})

# include GLFW headers and link against GLFW libraries
target_link_libraries(cg_vulkan PRIVATE glfw)

add_custom_command(
        TARGET cg_vulkan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:cg_vulkan>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/shaders/shader.vert.spv"
        "${CMAKE_BINARY_DIR}/shaders/shader.frag.spv"
        "$<TARGET_FILE_DIR:cg_vulkan>/shaders/"
)
