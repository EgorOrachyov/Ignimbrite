/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#include <Cache.h>

namespace ignimbrite {

    std::unordered_map<std::string, std::shared_ptr<CacheItem>> Cache::mCached;

    void Cache::removeItem(const std::string &name) {
        mCached.erase(name);
    }

    bool Cache::contains(const std::string &name) {
        auto found = mCached.find(name);
        return found != mCached.end();
    }

    bool Cache::addItem(std::shared_ptr<ignimbrite::CacheItem> item) {
        auto found = mCached.find(item->getCachedName());
        if (found == mCached.end()) {
            mCached[item->getCachedName()] = std::move(item);
            return true;
        }

        return false;
    }

    const std::shared_ptr<CacheItem>& Cache::getItem(const std::string &name) {
        return mCached.at(name);
    }

    void Cache::renameItem(const std::string &name, const std::string &newName) {
        if (contains(name)) {
            auto item = mCached[name];
            mCached.erase(name);
            mCached[newName] = std::move(item);
        }
    }

}