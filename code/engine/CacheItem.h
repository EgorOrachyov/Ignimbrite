/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_CACHEITEM_H
#define IGNIMBRITE_CACHEITEM_H

#include <Types.h>
#include <IncludeStd.h>

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
        void setCachedName(const String& name);
        /** @return Name of item, used to cache resource */
        const String& getCachedName() const;
    private:
        String mCachedName;
    };

}

#endif //IGNIMBRITE_CACHEITEM_H