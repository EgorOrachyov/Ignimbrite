//
// Created by Egor Orachyov on 2019-10-19.
//

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