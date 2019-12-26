/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITELIBRARY_OBJECTID_H
#define IGNIMBRITELIBRARY_OBJECTID_H

#include <ignimbrite/Types.h>

namespace ignimbrite {

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

    private:
        uint32 mIndex = 0;
        uint32 mGeneration = 0;
    };

} // namespace ignimbrite

#endif //IGNIMBRITELIBRARY_OBJECTID_H
