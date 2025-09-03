#pragma once

#include <string>
#include <vector>

class FileDialog {
public:
    struct Filter {
        std::string name;
        std::string spec;
    };
    
    static std::string openFile(const std::vector<Filter>& filters = {});
    static std::string saveFile(const std::vector<Filter>& filters = {});
    static std::string selectFolder();
};