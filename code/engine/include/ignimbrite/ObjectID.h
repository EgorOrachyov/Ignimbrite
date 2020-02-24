/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_OBJECTID_H
#define IGNIMBRITE_OBJECTID_H

#include <ignimbrite/Types.h>

namespace ignimbrite {

    /** Dummy placeholder for ObjectID */
    class DummyObject;

    /**
     * Unique id represented by index of the object  (index in array for robust access)
     * and generation value to know, whether ID was removed or not.
     *
     * ID is parametrised by type for type safe access
     * @tparam T type of the handle
     */
    template <typename T = DummyObject>
    struct ObjectID {
    public:
        ObjectID() = default;

        ObjectID(uint32 index, uint32 generation)
                : mIndex(index), mGeneration(generation) {
        }

        inline bool operator==(const ObjectID &other) const {
            return mIndex == other.mIndex &&
                   mGeneration == other.mGeneration;
        }

        inline bool operator!=(const ObjectID &other) const {
            return mIndex != other.mIndex ||
                   mGeneration != other.mGeneration;
        }

        inline uint32 getIndex() const {
            return mIndex;
        }

        inline uint32 getGeneration() const {
            return mGeneration;
        }

        inline bool isNull() const {
            return mIndex == 0 && mGeneration == 0;
        }

        inline bool isNotNull() const {
            return mIndex != 0 || mGeneration != 0;
        }

    private:
        uint32 mIndex = 0;
        uint32 mGeneration = 0;
    };

    template <typename T = DummyObject>
    using ID = ObjectID<T>;

} // namespace ignimbrite

#endif //IGNIMBRITE_OBJECTID_H