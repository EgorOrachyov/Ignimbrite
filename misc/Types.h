//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_TYPES_H
#define VULKANRENDERER_TYPES_H

#include <inttypes.h>

typedef float    float32;
typedef double   float64;

typedef wchar_t  wchar;

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

namespace TypeCheck {

#define SIZE_MESSAGE "Inappropriate type size for platform"

    static_assert(sizeof(float32) == 4, SIZE_MESSAGE);
    static_assert(sizeof(float64) == 8, SIZE_MESSAGE);

    static_assert(sizeof(int8)   == 1, SIZE_MESSAGE);
    static_assert(sizeof(int16)  == 2, SIZE_MESSAGE);
    static_assert(sizeof(int32)  == 4, SIZE_MESSAGE);
    static_assert(sizeof(int64)  == 8, SIZE_MESSAGE);

    static_assert(sizeof(uint8)  == 1, SIZE_MESSAGE);
    static_assert(sizeof(uint16) == 2, SIZE_MESSAGE);
    static_assert(sizeof(uint32) == 4, SIZE_MESSAGE);
    static_assert(sizeof(uint64) == 8, SIZE_MESSAGE);

#undef SIZE_MESSAGE

}

#endif //VULKANRENDERER_TYPES_H
