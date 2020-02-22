/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_CACHEITEM_H
#define IGNIMBRITE_CACHEITEM_H

#include <ignimbrite/Types.h>

namespace ignimbrite {

    class CacheItem {
    public:
        virtual ~CacheItem() = default;
    };

}

#endif //IGNIMBRITE_CACHEITEM_H