/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#include "AABB.h"

ignimbrite::AABB::AABB(const glm::vec3 &vec) {
    for (int i = 0; i < 3; i++) {
        minBounds[i] = maxBounds[i] = vec[i];
    }
}

ignimbrite::AABB::AABB(const glm::vec3 &center, const float radius) {
    for (int i = 0; i < 3; i++) {
        minBounds[i] = center[i] - radius;
        maxBounds[i] = center[i] + radius;
    }
}

ignimbrite::AABB::AABB(const glm::vec3 &vec1, const glm::vec3 &vec2) {
    for (int i = 0; i < 3; i++) {
        minBounds[i] = vec1[i] < vec2[i] ? vec1[i] : vec2[i];
        maxBounds[i] = vec1[i] > vec2[i] ? vec1[i] : vec2[i];
    }
}

bool ignimbrite::AABB::Contains(const ignimbrite::AABB &aabb) const {
    for (int i = 0; i < 3; i++) {
        if (aabb.minBounds[i] < minBounds[i] || aabb.maxBounds[i] > maxBounds[i]) {
            return false;
        }
    }

    return true;
}

bool ignimbrite::AABB::Contains(const glm::vec3 &point) const {
    for (int i = 0; i < 3; i++)  {
        if (point[i] < minBounds[i] || point[i] > maxBounds[i]) {
            return false;
        }
    }

    return true;
}

bool ignimbrite::AABB::Intersect(const ignimbrite::AABB &other) {
    const auto &mina = minBounds;
    const auto &maxa = maxBounds;
    const auto &minb = other.minBounds;
    const auto &maxb = other.maxBounds;

    for (int i = 0; i < 3; i++)
    {
        if (maxa[i] < minb[i] || mina[i] > maxb[i])
        {
            return false;
        }
    }

    return true;
}

