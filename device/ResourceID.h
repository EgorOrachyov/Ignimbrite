//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_RESOURCEID_H
#define VULKANRENDERER_RESOURCEID_H

/**
 * Int64 based Object id.
 * Allows to identify object via index in buffer and generation.
 */
class ResourceID {
public:
    ResourceID() = default;
    ResourceID(long index, long generation)
        : mIndex(index),
          mGeneration(generation) {
    }
    bool operator==(const ResourceID& other) const {
        return mIndex == other.mIndex &&
            mGeneration == other.mGeneration;
    }
    bool operator!=(const ResourceID& other) const {
        return mIndex != other.mIndex ||
            mGeneration != other.mGeneration;
    }
    long getIndex() const {
        return mIndex;
    }
    long getGeneration() const {
        return mGeneration;
    }
public:
    static const long INVALID_VALUE = 0xffffffffffffffff;
    static const ResourceID INVALID_ID;
private:
    friend class ObjectIDBuffer;
    long mIndex = INVALID_VALUE;
    long mGeneration = INVALID_VALUE;
};

const ResourceID ResourceID::INVALID_ID = ResourceID(INVALID_VALUE, INVALID_VALUE);

#endif //VULKANRENDERER_RESOURCEID_H
