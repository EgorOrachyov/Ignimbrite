/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_CACHE_H
#define IGNIMBRITE_CACHE_H

#include <CacheItem.h>
#include <IncludeStd.h>

namespace ignimbrite {

    class Cache {
    public:
        static void removeItem(const String& name);
        static bool contains(const String& name);
        static bool addItem(RefCounted<CacheItem> item);
        static const RefCounted<CacheItem> &getItem(const String& name);
    private:
        friend class CacheItem;
        static void renameItem(const String& name, const String& newName);
        static std::unordered_map<String, RefCounted<CacheItem>> mCached;
    };

}

#endif //IGNIMBRITE_CACHE_H