//
// Created by Egor Orachyov on 2019-10-19.
//

#ifndef VULKANRENDERER_FILEUTILS_H
#define VULKANRENDERER_FILEUTILS_H

#include <vector>
#include <string>

class FileUtils {
public:
    static void loadData(const std::string &filename, std::vector<char> &data);
};

#endif //VULKANRENDERER_FILEUTILS_H