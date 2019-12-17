//
// Created by Egor Orachyov on 2019-10-19.
//

#ifndef VULKANRENDERER_FILEUTILS_H
#define VULKANRENDERER_FILEUTILS_H

#include <vector>
#include <string>

namespace ignimbrite {

    class FileUtils {
    public:
        static void loadData(const std::string &filename, std::vector<char> &data);
    };

} // namespace ignimbrite

#endif //VULKANRENDERER_FILEUTILS_H