//
// Created by Egor Orachyov on 2019-10-26.
//

#ifndef VULKANRENDERER_OBJECTIDBUFFER_H
#define VULKANRENDERER_OBJECTIDBUFFER_H

#include <ignimbrite/ObjectID.h>
#include <ignimbrite/Compilation.h>
#include <vector>
#include <iostream>
#include <new>

namespace ignimbrite {

/**
 * ID indexed buffer. Allows to access objects via unique ID in O(1).
 * Supported operations: add, get, remove.
 *
 * @note Not thread safe
 * @tparam T Type of stored objects
 */
    template<typename T>
    class ObjectIDBuffer {
    public:
        ObjectIDBuffer();
        ~ObjectIDBuffer();

        ObjectID add(const T &object);

        /**
         * @brief Moves object into container.
         * @note Old object references becomes invalid.
         * Preferred for use, because this method is more optimal
         * and allows you not to copy object, instead it moves (std::move)
         * object into this buffer memory
         * @return Object ID in the buffer
         */
        ObjectID move(T &object);

        T &get(ObjectID id) const;
        T *getPtr(ObjectID id) const;

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

    public:

        class Iterator {
        public:
            Iterator(uint32 current, const std::vector<RawObject> &objects, const std::vector<uint32> &gens,
                     const std::vector<uint32> &freeGensIndices);

            bool operator!=(const Iterator &other);

            T &operator*();

            void operator++();

            ObjectID getID();

        private:
            T *object = nullptr;
            bool found = false;
            uint32 current;
            ObjectID id;
            const std::vector<RawObject> &objects;
            const std::vector<uint32> &gens;
            const std::vector<uint32> &freeGensIndices;
        };

        Iterator begin();

        Iterator end();

    };

    template<typename T>
    ObjectIDBuffer<T>::ObjectIDBuffer() {

    }

    template<typename T>
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
                T *object = (T *) &mObjects[i];
                object->~T();
            }
        }
#endif
    }

    template<typename T>
    ObjectID ObjectIDBuffer<T>::add(const T &object) {
        uint32 index;
        uint32 generation;

        if (mFreeIDs == 0) {
            index = mGens.size();
            generation = INITIAL_GENERATION;

            mGens.push_back(generation);
            mObjects.push_back(RawObject());
        } else {
            index = mFreeGensIndices.back();
            mFreeGensIndices.pop_back();
            generation = mGens[index] + 1;

            mFreeIDs -= 1;
        }

        void *memory = &mObjects[index];
        new(memory) T(object);

        mUsedIDs += 1;

        return {index, generation};
    }

    template<typename T>
    ObjectID ObjectIDBuffer<T>::move(T &object) {
        uint32 index;
        uint32 generation;

        if (mFreeIDs == 0) {
            index = mGens.size();
            generation = INITIAL_GENERATION;

            mGens.push_back(generation);
            mObjects.push_back(RawObject());
        } else {
            index = mFreeGensIndices.back();
            mFreeGensIndices.pop_back();
            generation = mGens[index] + 1;

            mFreeIDs -= 1;
        }

        void *memory = &mObjects[index];
        new(memory) T(std::move(object));

        mUsedIDs += 1;

        return {index, generation};
    }

    template<typename T>
    T &ObjectIDBuffer<T>::get(ObjectID id) const {
        T *object = getPtr(id);

        if (object != nullptr) {
            return *object;
        } else {
            throw std::runtime_error("No object with specified id");
        }
    }

    template<typename T>
    T *ObjectIDBuffer<T>::getPtr(ObjectID id) const {
        uint32 index = id.getIndex();
        uint32 generation = id.getGeneration();

        if (index >= mGens.size()) {
            return nullptr;
        }

        if (generation != mGens[index]) {
            return nullptr;
        }

        return (T *) &mObjects[index];
    }

    template<typename T>
    bool ObjectIDBuffer<T>::contains(ObjectID id) const {
        return getPtr(id) != nullptr;
    }

    template<typename T>
    void ObjectIDBuffer<T>::remove(ObjectID id) {
        T *object = getPtr(id);

        if (object == nullptr) {
#ifdef MODE_DEBUG
            throw std::runtime_error("An attempt to remove unknown object");
#endif
        } else {
            uint32 index = id.getIndex();

            mGens[index] += 1;
            mFreeGensIndices.push_back(index);
            object->~T();

            mUsedIDs -= 1;
            mFreeIDs += 1;
        }
    }

    template<typename T>
    uint32 ObjectIDBuffer<T>::getNumUsedIDs() const {
        return mUsedIDs;
    }

    template<typename T>
    uint32 ObjectIDBuffer<T>::getNumFreeIDs() const {
        return mFreeIDs;
    }

    template<typename T>
    typename ObjectIDBuffer<T>::Iterator ObjectIDBuffer<T>::begin() {
        return Iterator(0, mObjects, mGens, mFreeGensIndices);
    }

    template<typename T>
    typename ObjectIDBuffer<T>::Iterator ObjectIDBuffer<T>::end() {
        return Iterator(mGens.size(), mObjects, mGens, mFreeGensIndices);
    }

    template<typename T>
    ObjectIDBuffer<T>::Iterator::Iterator(uint32 current, const std::vector<ObjectIDBuffer::RawObject> &objects,
                                          const std::vector<uint32> &gens, const std::vector<uint32> &freeGensIndices)
            : current(current), objects(objects), gens(gens), freeGensIndices(freeGensIndices) {
        operator++();
    }

    template<typename T>
    bool ObjectIDBuffer<T>::Iterator::operator!=(const ObjectIDBuffer<T>::Iterator &other) {
        return object != other.object;
    }

    template<typename T>
    T &ObjectIDBuffer<T>::Iterator::operator*() {
        return *object;
    }

    template<typename T>
    void ObjectIDBuffer<T>::Iterator::operator++() {
        found = false;

        for (; current < gens.size(); current++) {
            bool isValid = true;
            for (auto notUsed: freeGensIndices) {
                if (notUsed == current) {
                    isValid = false;
                    break;
                }
            }

            if (isValid) {
                found = true;
                object = (T *) &objects[current];
                id = ObjectID(current, gens[current]);
                break;
            }
        }

        if (!found) {
            object = nullptr;
        } else {
            current += 1;
        }
    }

    template<typename T>
    ObjectID ObjectIDBuffer<T>::Iterator::getID() {
        return id;
    }

} // namespace ignimbrite

#endif //VULKANRENDERER_OBJECTIDBUFFER_H