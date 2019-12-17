//
// Created by Egor Orachyov on 2019-10-26.
//

#include <ignimbrite/FileUtils.h>
#include <fstream>

namespace ignimbrite {

    void FileUtils::loadData(const std::string &filename, std::vector<char> &data) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file");
        }

        size_t size = (size_t) file.tellg();
        data.resize(size);
        file.seekg(0);
        file.read(data.data(), size);
    }

} // namespace ignimbrite