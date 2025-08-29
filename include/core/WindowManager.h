#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class WindowManager {
public:
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

private:
    GLFWwindow* m_window;
    uint32_t m_width;
    uint32_t m_height;
    bool m_framebufferResized = false;

    static void glfwErrorCallback(int error, const char* description);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

#endif // WINDOW_MANAGER_H
