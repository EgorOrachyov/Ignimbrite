/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Cache.h>

namespace ignimbrite {

    std::unordered_map<String, RefCounted<CacheItem>> Cache::mCached;

    void Cache::removeItem(const String &name) {
        mCached.erase(name);
    }

    bool Cache::contains(const String &name) {
        auto found = mCached.find(name);
        return found != mCached.end();
    }

    bool Cache::addItem(RefCounted<ignimbrite::CacheItem> item) {
        auto found = mCached.find(item->getCachedName());
        if (found == mCached.end()) {
            mCached[item->getCachedName()] = std::move(item);
            return true;
        }

        return false;
    }

    const RefCounted<CacheItem>& Cache::getItem(const String &name) {
        return mCached.at(name);
    }

    void Cache::renameItem(const String &name, const String &newName) {
        if (contains(name)) {
            auto item = mCached[name];
            mCached.erase(name);
            mCached[newName] = std::move(item);
        }
    }

}