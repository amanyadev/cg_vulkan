#include "utils/EXRLoader.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// EXR support temporarily disabled due to TinyEXR dependency issues
// TODO: Fix TinyEXR integration
// #define TINYEXR_IMPLEMENTATION
// #define TINYEXR_USE_MINIZ 1
// #define TINYEXR_USE_THREAD 0
// #include <tinyexr.h>

bool HDRImage::loadFromFile(const std::string& filePath) {
    return EXRLoader::loadEXR(filePath, *this);
}

glm::vec3 HDRImage::getPixelRGB(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height || data.empty()) {
        return glm::vec3(0.0f);
    }
    
    int index = (y * width + x) * channels;
    if (channels >= 3) {
        return glm::vec3(data[index], data[index + 1], data[index + 2]);
    } else if (channels == 1) {
        float val = data[index];
        return glm::vec3(val, val, val);
    }
    return glm::vec3(0.0f);
}

glm::vec4 HDRImage::getPixelRGBA(int x, int y) const {
    glm::vec3 rgb = getPixelRGB(x, y);
    float alpha = 1.0f;
    
    if (channels >= 4) {
        int index = (y * width + x) * channels;
        alpha = data[index + 3];
    }
    
    return glm::vec4(rgb, alpha);
}

std::vector<uint8_t> HDRImage::tonemapToLDR(float exposure, float gamma) const {
    std::vector<uint8_t> ldrData;
    ldrData.reserve(width * height * 4); // RGBA
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec4 hdrPixel = getPixelRGBA(x, y);
            
            // Apply exposure
            hdrPixel.r = EXRLoader::tonemap(hdrPixel.r * exposure, 1.0f);
            hdrPixel.g = EXRLoader::tonemap(hdrPixel.g * exposure, 1.0f);  
            hdrPixel.b = EXRLoader::tonemap(hdrPixel.b * exposure, 1.0f);
            
            // Gamma correction and convert to 8-bit
            ldrData.push_back(EXRLoader::gammaCorrect(hdrPixel.r, gamma));
            ldrData.push_back(EXRLoader::gammaCorrect(hdrPixel.g, gamma));
            ldrData.push_back(EXRLoader::gammaCorrect(hdrPixel.b, gamma));
            ldrData.push_back(static_cast<uint8_t>(std::clamp(hdrPixel.a * 255.0f, 0.0f, 255.0f)));
        }
    }
    
    return ldrData;
}

bool EXRLoader::isEXRFile(const std::string& filePath) {
    std::string ext = filePath.substr(filePath.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "exr";
}

bool EXRLoader::loadEXR(const std::string& filePath, HDRImage& outImage) {
    std::cout << "EXR loading is currently disabled - TinyEXR dependency issues" << std::endl;
    std::cout << "EXR file: " << filePath << " will be skipped" << std::endl;
    return false;
    
    // TODO: Fix TinyEXR integration
    /* 
    float* rgba = nullptr;
    int width, height;
    const char* err = nullptr;
    
    // Load EXR using TinyEXR
    int ret = LoadEXR(&rgba, &width, &height, filePath.c_str(), &err);
    
    if (ret != TINYEXR_SUCCESS) {
        if (err) {
            std::cerr << "Error loading EXR file '" << filePath << "': " << err << std::endl;
            FreeEXRErrorMessage(err);
        } else {
            std::cerr << "Failed to load EXR file: " << filePath << std::endl;
        }
        return false;
    }
    
    outImage.width = width;
    outImage.height = height;
    outImage.channels = 4; // RGBA
    
    std::cout << "EXR dimensions: " << outImage.width << "x" << outImage.height << std::endl;
    
    // Copy data to our format
    size_t totalPixels = width * height * 4;
    outImage.data.clear();
    outImage.data.reserve(totalPixels);
    
    for (size_t i = 0; i < totalPixels; ++i) {
        outImage.data.push_back(rgba[i]);
    }
    
    // Free TinyEXR allocated memory
    free(rgba);
    
    std::cout << "Successfully loaded EXR file with " << outImage.data.size() << " float values" << std::endl;
    return true;
    */
}

float EXRLoader::tonemap(float value, float exposure) {
    // Simple Reinhard tone mapping
    float exposed = value * exposure;
    return exposed / (1.0f + exposed);
}

uint8_t EXRLoader::gammaCorrect(float value, float gamma) {
    float corrected = std::pow(std::clamp(value, 0.0f, 1.0f), 1.0f / gamma);
    return static_cast<uint8_t>(corrected * 255.0f);
}