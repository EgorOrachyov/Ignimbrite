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
#include <string>

namespace ignimbrite {

    /** Something, what can be placed in cache and accessed via simple name */
    class CacheItem {
    public:
        virtual ~CacheItem() = default;
        const std::string& getCachedName() const { return mCachedName; }
    protected:
        std::string mCachedName;
    };

}

#endif //IGNIMBRITE_CACHEITEM_H