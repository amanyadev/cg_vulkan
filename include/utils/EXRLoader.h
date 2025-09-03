#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct HDRImage {
    int width;
    int height;
    int channels;
    std::vector<float> data; // HDR float data
    
    bool loadFromFile(const std::string& filePath);
    
    // Get pixel data in different formats
    glm::vec3 getPixelRGB(int x, int y) const;
    glm::vec4 getPixelRGBA(int x, int y) const;
    
    // Convert to LDR for display
    std::vector<uint8_t> tonemapToLDR(float exposure = 1.0f, float gamma = 2.2f) const;
};

class EXRLoader {
public:
    static bool isEXRFile(const std::string& filePath);
    static bool loadEXR(const std::string& filePath, HDRImage& outImage);
    static float tonemap(float value, float exposure);
    static uint8_t gammaCorrect(float value, float gamma);
};