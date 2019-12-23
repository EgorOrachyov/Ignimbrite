/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_FILEUTILS_H
#define IGNIMBRITELIBRARY_FILEUTILS_H

#include <vector>
#include <string>

namespace ignimbrite {

    class FileUtils {
    public:
        static void loadData(const std::string &filename, std::vector<char> &data);
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_FILEUTILS_H