//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef VULKANRENDERER_OBJECTID_H
#define VULKANRENDERER_OBJECTID_H

#include <Types.h>

/**
 * Unique id represented by index of the object
 * (index in array for robust access) and generation
 * value to know, whether ID was removed or not.
 */
struct ObjectID {
public:
    ObjectID() = default;
    ObjectID(uint32 index, uint32 generation)
        : mIndex(index), mGeneration(generation) {
    }
    inline bool operator==(const ObjectID& other) const {
        return mIndex == other.mIndex &&
               mGeneration == other.mGeneration;
    }
    inline bool operator!=(const ObjectID& other) const {
        return mIndex != other.mIndex ||
               mGeneration != other.mGeneration;
    }
    inline uint32 getIndex() const {
        return mIndex;
    }
    inline uint32 getGeneration() const {
        return mGeneration;
    }
private:
    uint32 mIndex = 0;
    uint32 mGeneration = 0;
};

#endif //VULKANRENDERER_OBJECTID_H
