//
// Created by Aman Yadav on 6/19/25.
//

#ifndef VULKANAPP_H
#define VULKANAPP_H
#include <cstdint>

class VulkanApp {
public:
    void run();

private:
    const uint32_t WIDTH = 640;
    const uint32_t HEIGHT = 480;
    void initVulkan();
    void mainLoop();
    void cleanup();
    void initWindow();

    void createInstance();
};

#endif // VULKANAPP_H
