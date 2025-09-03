#include "utils/FileDialog.h"
#include <nfd.h>
#include <iostream>

std::string FileDialog::openFile(const std::vector<Filter>& filters) {
    // Convert filters to NFD format
    std::vector<nfdfilteritem_t> nfdFilters;
    for (const auto& filter : filters) {
        nfdFilters.push_back({filter.name.c_str(), filter.spec.c_str()});
    }
    
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_OpenDialog(&outPath,
        nfdFilters.empty() ? nullptr : nfdFilters.data(),
        nfdFilters.size(),
        nullptr);
    
    if (result == NFD_OKAY) {
        std::string path(outPath);
        free(outPath);  // NFD requires manual memory management
        return path;
    } else if (result == NFD_CANCEL) {
        // User pressed cancel
        return "";
    } else {
        std::cerr << "File dialog error: " << NFD_GetError() << std::endl;
        return "";
    }
}

std::string FileDialog::saveFile(const std::vector<Filter>& filters) {
    // Convert filters to NFD format
    std::vector<nfdfilteritem_t> nfdFilters;
    for (const auto& filter : filters) {
        nfdFilters.push_back({filter.name.c_str(), filter.spec.c_str()});
    }
    
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_SaveDialog(&outPath,
        nfdFilters.empty() ? nullptr : nfdFilters.data(),
        nfdFilters.size(),
        nullptr,
        nullptr);
    
    if (result == NFD_OKAY) {
        std::string path(outPath);
        free(outPath);  // NFD requires manual memory management
        return path;
    } else if (result == NFD_CANCEL) {
        // User pressed cancel
        return "";
    } else {
        std::cerr << "File dialog error: " << NFD_GetError() << std::endl;
        return "";
    }
}

std::string FileDialog::selectFolder() {
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
    
    if (result == NFD_OKAY) {
        std::string path(outPath);
        free(outPath);  // NFD requires manual memory management
        return path;
    } else if (result == NFD_CANCEL) {
        // User pressed cancel
        return "";
    } else {
        std::cerr << "File dialog error: " << NFD_GetError() << std::endl;
        return "";
    }
}