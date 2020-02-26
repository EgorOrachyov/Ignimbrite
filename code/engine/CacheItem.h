/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_CACHEITEM_H
#define IGNIMBRITE_CACHEITEM_H

#include <Types.h>
#include <StdIncludes.h>

namespace ignimbrite {

    /** Something, what can be placed in cache and accessed via simple name */
    class CacheItem {
    public:
        virtual ~CacheItem() = default;
        /** @return True, if this item in cache */
        bool isCached();
        /** Remove this item from cache */
        void removeFromCache();
        /** Renames item, updates its name in cache if needed */
        void setCachedName(const std::string& name);
        /** @return Name of item, used to cache resource */
        const std::string& getCachedName() const;
    protected:
        std::string mCachedName;
    };

}

#endif //IGNIMBRITE_CACHEITEM_H