/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_INCLUDESTD_H
#define IGNIMBRITE_INCLUDESTD_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace ignimbrite {
    /** Place type aliases here */
    using String = std::string;

    template <typename T>
    using RefCounted = std::shared_ptr<T>;
}

#endif //IGNIMBRITE_INCLUDESTD_H
