//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef VULKANRENDERER_OBJECTIDBUFFER_H
#define VULKANRENDERER_OBJECTIDBUFFER_H

#include "ObjectID.h"
#include <vector>
#include "Compilation.h"
#include <iostream>
#include <new>

/**
 * ID indexed buffer. Allows to access objects via unique ID in O(1).
 * Supported operations: add, get, remove.
 *
 * @note Not thread safe
 * @tparam T Type of stored objects
 */
template <typename T>
class ObjectIDBuffer {
public:
    ObjectIDBuffer();
    ~ObjectIDBuffer();
    ObjectID add(const T& object);
    T& get(ObjectID id) const;
    T* getPtr(ObjectID id) const;
    void remove(ObjectID id);
    bool contains(ObjectID id) const;
    uint32 getNumUsedIDs() const;
    uint32 getNumFreeIDs() const;
private:
    struct RawObject {
        uint8 mem[sizeof(T)];
    };

    static const uint32 INITIAL_GENERATION = 0x1;

    std::vector<RawObject> mObjects;
    std::vector<uint32> mGens;
    std::vector<uint32> mFreeGensIndices;

    uint32 mFreeIDs = 0;
    uint32 mUsedIDs = 0;
};

template <typename T>
ObjectIDBuffer<T>::ObjectIDBuffer() {

}

template <typename T>
ObjectIDBuffer<T>::~ObjectIDBuffer() {
    if (mUsedIDs != 0) {
        std::cout << "ObjectIDBuffer: all objects must be explicitly removed [count: " << mUsedIDs << "]\n";
    }

#ifdef MODE_DEBUG
    for (uint32 i = 0; i < mGens.size(); i++) {
        bool wasRemoved = false;
        for (uint32 j: mFreeGensIndices) {
            if (i == j) {
                wasRemoved = true;
                break;
            }
        }

        if (!wasRemoved) {
            std::cout << "ObjectIDBuffer: lost id: (" << i << "," << mGens[i] << ")\n";
            T* object = (T*)& mObjects[i];
            object->~T();
        }
    }
#endif
}

template <typename T>
ObjectID ObjectIDBuffer<T>::add(const T &object) {
    uint32 index;
    uint32 generation;

    if (mFreeIDs == 0) {
        index = mGens.size();
        generation = INITIAL_GENERATION;

        mGens.push_back(generation);
        mObjects.push_back(RawObject());
    }
    else {
        index = mFreeGensIndices.back();
        mFreeGensIndices.pop_back();
        generation = mGens[index] + 1;

        mFreeIDs -= 1;
    }

    void* memory = &mObjects[index];
    new (memory) T(object);

    mUsedIDs += 1;

    return { index, generation };
}

template <typename T>
T& ObjectIDBuffer<T>::get(ObjectID id) const {
    T* object = getPtr(id);

    if (object != nullptr) {
        return *object;
    } else {
        throw std::runtime_error("No object with specified id");
    }
}

template <typename T>
T* ObjectIDBuffer<T>::getPtr(ObjectID id) const {
    uint32 index = id.getIndex();
    uint32 generation = id.getGeneration();

    if (index >= mGens.size()) {
        return nullptr;
    }

    if (generation != mGens[index]) {
        return nullptr;
    }

    return (T*) &mObjects[index];
}

template <typename T>
bool ObjectIDBuffer<T>::contains(ObjectID id) const {
    return getPtr(id) != nullptr;
}

template <typename T>
void ObjectIDBuffer<T>::remove(ObjectID id) {
    T* object = getPtr(id);

    if (object == nullptr) {
#ifdef MODE_DEBUG
        throw std::runtime_error("An attempt to remove unknown object");
#endif
    }
    else {
        uint32 index = id.getIndex();

        mGens[index] += 1;
        mFreeGensIndices.push_back(index);
        object->~T();

        mUsedIDs -= 1;
        mFreeIDs += 1;
    }
}

template <typename T>
uint32 ObjectIDBuffer<T>::getNumUsedIDs() const {
    return mUsedIDs;
}

template <typename T>
uint32 ObjectIDBuffer<T>::getNumFreeIDs() const {
    return mFreeIDs;
}

#endif //VULKANRENDERER_OBJECTIDBUFFER_H