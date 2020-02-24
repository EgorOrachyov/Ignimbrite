/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_OBJECTIDBUFFER_H
#define IGNIMBRITE_OBJECTIDBUFFER_H

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
     *
     * @tparam T Type of stored objects
     * @tparam H Type of object ids for users
     */
    template<typename T, typename H = DummyObject>
    class ObjectIDBuffer {
    public:
        ObjectIDBuffer() = default;
        ~ObjectIDBuffer();

        ObjectID<H> add(const T &object);

        /**
         * @brief Moves object into container.
         * @note Old object references becomes invalid.
         * Preferred for use, because this method is more optimal
         * and allows you not to copy object, instead it moves (std::move)
         * object into this buffer memory
         * @return Object ID in the buffer
         */
        ObjectID<H> move(T &object);

        T &get(ObjectID<H> id) const;
        T *getPtr(ObjectID<H> id) const;

        void remove(ObjectID<H> id);
        bool contains(ObjectID<H> id) const;

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
            void operator++();
            T &operator*();

            ObjectID<H> getID();

        private:
            T *object = nullptr;
            bool found = false;
            uint32 current;
            ObjectID<H> id;
            const std::vector<RawObject> &objects;
            const std::vector<uint32> &gens;
            const std::vector<uint32> &freeGensIndices;
        };

        Iterator begin();

        Iterator end();

    };

    template <typename T, typename H = DummyObject>
    using IDBuffer = ObjectIDBuffer<T,H>;

    template<typename T,typename H>
    ObjectIDBuffer<T,H>::~ObjectIDBuffer() {
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

    template<typename T,typename H>
    ObjectID<H> ObjectIDBuffer<T,H>::add(const T &object) {
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
            generation = mGens[index];

            mFreeIDs -= 1;
        }

        void *memory = &mObjects[index];
        new(memory) T(object);

        mUsedIDs += 1;

        return {index, generation};
    }

    template<typename T,typename H>
    ObjectID<H> ObjectIDBuffer<T,H>::move(T &object) {
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
            generation = mGens[index];

            mFreeIDs -= 1;
        }

        void *memory = &mObjects[index];
        new(memory) T(std::move(object));

        mUsedIDs += 1;

        return {index, generation};
    }

    template<typename T,typename H>
    T &ObjectIDBuffer<T,H>::get(ObjectID<H> id) const {
        T *object = getPtr(id);

        if (object != nullptr) {
            return *object;
        } else {
            throw std::runtime_error("No object with specified id");
        }
    }

    template<typename T,typename H>
    T *ObjectIDBuffer<T,H>::getPtr(ObjectID<H> id) const {
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

    template<typename T,typename H>
    bool ObjectIDBuffer<T,H>::contains(ObjectID<H> id) const {
        return getPtr(id) != nullptr;
    }

    template<typename T,typename H>
    void ObjectIDBuffer<T,H>::remove(ObjectID<H> id) {
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

    template<typename T,typename H>
    uint32 ObjectIDBuffer<T,H>::getNumUsedIDs() const {
        return mUsedIDs;
    }

    template<typename T,typename H>
    uint32 ObjectIDBuffer<T,H>::getNumFreeIDs() const {
        return mFreeIDs;
    }

    template<typename T,typename H>
    typename ObjectIDBuffer<T,H>::Iterator ObjectIDBuffer<T,H>::begin() {
        return Iterator(0, mObjects, mGens, mFreeGensIndices);
    }

    template<typename T,typename H>
    typename ObjectIDBuffer<T,H>::Iterator ObjectIDBuffer<T,H>::end() {
        return Iterator(mGens.size(), mObjects, mGens, mFreeGensIndices);
    }

    template<typename T,typename H>
    ObjectIDBuffer<T,H>::Iterator::Iterator(uint32 current, const std::vector<ObjectIDBuffer::RawObject> &objects,
                                          const std::vector<uint32> &gens, const std::vector<uint32> &freeGensIndices)
            : current(current), objects(objects), gens(gens), freeGensIndices(freeGensIndices) {
        operator++();
    }

    template<typename T,typename H>
    bool ObjectIDBuffer<T,H>::Iterator::operator!=(const ObjectIDBuffer<T,H>::Iterator &other) {
        return object != other.object;
    }

    template<typename T,typename H>
    T &ObjectIDBuffer<T,H>::Iterator::operator*() {
        return *object;
    }

    template<typename T,typename H>
    void ObjectIDBuffer<T,H>::Iterator::operator++() {
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
                id = ObjectID<H>(current, gens[current]);
                break;
            }
        }

        if (!found) {
            object = nullptr;
        } else {
            current += 1;
        }
    }

    template<typename T,typename H>
    ObjectID<H> ObjectIDBuffer<T,H>::Iterator::getID() {
        return id;
    }

} // namespace ignimbrite

#endif //IGNIMBRITE_OBJECTIDBUFFER_H
