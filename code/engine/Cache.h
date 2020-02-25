/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_CACHE_H
#define IGNIMBRITE_CACHE_H

#include <CacheItem.h>
#include <unordered_map>
#include <memory>

namespace ignimbrite {

    class Cache {
    public:
        static void removeItem(const std::string& name);
        static bool contains(const std::string& name);
        static bool addItem(std::shared_ptr<CacheItem> item);
        static const std::shared_ptr<CacheItem> &getItem(const std::string& name);
    private:
        friend class CacheItem;
        static void renameItem(const std::string& name, const std::string& newName);
        static std::unordered_map<std::string, std::shared_ptr<CacheItem>> mCached;
    };

}

#endif //IGNIMBRITE_CACHE_H