/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_AABB_H
#define IGNIMBRITE_AABB_H

#include <glm/glm.hpp>
#include <Types.h>

namespace ignimbrite {

    /**
     * @brief Axis aligned bounding box
     * Bounding box is used for culling operations on rendered objects
     */
    class AABB {
    public:
        /** Null aabb */
        AABB() = default;

        /** Create bounding box Min=-vec, Max=vec */
        AABB(const glm::vec3 &vec) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = maxBounds[i] = vec[i];
            }
        }

        /** Create bounding box of sphere */
        AABB(const glm::vec3 &center, float radius) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = center[i] - radius;
                maxBounds[i] = center[i] + radius;
            }
        }

        /** Create bounding box of two arbitary vectors */
        AABB(const glm::vec3 &vec1, const glm::vec3 &vec2) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = vec1[i] < vec2[i] ? vec1[i] : vec2[i];
                maxBounds[i] = vec1[i] > vec2[i] ? vec1[i] : vec2[i];
            }
        }

        /** Does this AABB contain other one? */
        bool contains(const AABB &aabb) const {
            for (int i = 0; i < 3; i++) {
                if (aabb.minBounds[i] < minBounds[i] || aabb.maxBounds[i] > maxBounds[i]) {
                    return false;
                }
            }

            return true;
        }

        /** Does this AABB contain a point? */
        bool contains(const glm::vec3 &point) const {
            for (int i = 0; i < 3; i++)  {
                if (point[i] < minBounds[i] || point[i] > maxBounds[i]) {
                    return false;
                }
            }

            return true;
        }

        /** Is there intersection between this and other AABB */
        bool intersect(const AABB &other) {
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

        // TODO: plane intersections
    private:
        glm::vec3 minBounds = glm::vec3(0.0f);
        glm::vec3 maxBounds = glm::vec3(0.0f);
    };

}

#endif //IGNIMBRITE_AABB_H