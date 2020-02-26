/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <CacheItem.h>
#include <Cache.h>

namespace ignimbrite {

    bool CacheItem::isCached() {
        return Cache::contains(mCachedName);
    }

    void CacheItem::removeFromCache() {
        if (isCached()) {
            Cache::removeItem(mCachedName);
        }
    }

    void CacheItem::setCachedName(const String &name) {
        if (isCached()) {
            Cache::renameItem(mCachedName, name);
        }
        else {
            mCachedName = name;
        }
    }

    const String& CacheItem::getCachedName() const {
        return mCachedName;
    }

}