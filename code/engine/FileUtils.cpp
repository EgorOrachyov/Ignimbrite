/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <FileUtils.h>
#include <fstream>

namespace ignimbrite {

    void FileUtils::loadData(const String &filename, std::vector<char> &data) {
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