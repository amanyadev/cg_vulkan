#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>

class WindowManager {
public:
    // Input callback function types
    using MouseMoveCallback = std::function<void(double, double)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;
    using ScrollCallback = std::function<void(double, double)>;
    using KeyCallback = std::function<void(int, int, int, int)>;
    using DropCallback = std::function<void(int, const char**)>;

    WindowManager(uint32_t width, uint32_t height, const char* title);
    ~WindowManager();

    GLFWwindow* getWindow() const { return m_window; }
    bool shouldClose() const;
    void pollEvents() const;

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    
    bool wasResized() const { return m_framebufferResized; }
    void resetResizeFlag() { m_framebufferResized = false; }
    void updateSize(uint32_t width, uint32_t height) { m_width = width; m_height = height; }
    
    // Mouse control functions
    void getMousePosition(double* xpos, double* ypos) const;
    void setMousePosition(double xpos, double ypos) const;
    bool isMouseButtonPressed(int button) const;
    void setCursorMode(int mode) const;
    
    // Input callback registration
    void setMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void setScrollCallback(ScrollCallback callback) { m_scrollCallback = callback; }
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void setDropCallback(DropCallback callback) { m_dropCallback = callback; }

private:
    GLFWwindow* m_window;
    uint32_t m_width;
    uint32_t m_height;
    bool m_framebufferResized = false;
    
    // Input callbacks
    MouseMoveCallback m_mouseMoveCallback;
    MouseButtonCallback m_mouseButtonCallback;
    ScrollCallback m_scrollCallback;
    KeyCallback m_keyCallback;
    DropCallback m_dropCallback;

    static void glfwErrorCallback(int error, const char* description);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    // GLFW input callbacks
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void dropCallback(GLFWwindow* window, int count, const char** paths);
};

#endif // WINDOW_MANAGER_H
