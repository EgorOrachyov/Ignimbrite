/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_FILEUTILS_H
#define IGNIMBRITE_FILEUTILS_H

#include <Types.h>
#include <IncludeStd.h>

namespace ignimbrite {

    class FileUtils {
    public:
        static void loadData(const String &filename, std::vector<char> &data);
        static void loadBinary(const String& filename, std::vector<uint8> &data);
    };

} // namespace ignimbrite

#endif //IGNIMBRITE_FILEUTILS_H