//
// Created by Egor Orachyov on 2019-10-19.
//

#ifndef VULKANRENDERER_FILEUTILS_H
#define VULKANRENDERER_FILEUTILS_H

#include <fstream>
#include <vector>
#include <string>

class FileUtils {
public:
    static void loadData(const std::string &filename, std::vector<char> &data) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file");
        }

        size_t size = (size_t) file.tellg();
        data.resize(size);
        file.seekg(0);
        file.read(data.data(), size);
    }
};


#endif //VULKANRENDERER_FILEUTILS_H
