//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_OBJECTID_H
#define VULKANRENDERER_OBJECTID_H

/**
 * Int64 based Object id.
 * Allows to identify object via index in buffer and generation.
 */
class ObjectID {
public:
    ObjectID() = default;
    ObjectID(long index, long generation)
        : mIndex(index),
          mGeneration(generation) {
    }
    bool operator==(const ObjectID& other) const {
        return mIndex == other.mIndex &&
            mGeneration == other.mGeneration;
    }
    bool operator!=(const ObjectID& other) const {
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
    static const ObjectID INVALID_ID;
private:
    friend class ObjectIDBuffer;
    long mIndex = INVALID_VALUE;
    long mGeneration = INVALID_VALUE;
};

const ObjectID ObjectID::INVALID_ID = ObjectID(INVALID_VALUE, INVALID_VALUE);

#endif //VULKANRENDERER_OBJECTID_H
