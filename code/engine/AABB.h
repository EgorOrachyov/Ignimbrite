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
#include <glm/vec3.hpp>
#include <Types.h>
#include <array>

namespace ignimbrite {

    /**
     * @brief Axis aligned bounding box
     * Bounding box is used for culling operations on rendered objects
     */
    class AABB {
    public:
        /** Null aabb */
        AABB() = default;

        /** AABB from oriented 8 vertices volume box */
        AABB(const std::array<glm::vec3, 8> &box) {
            for (const auto& v: box) {
                for (uint32 i = 0; i < 3; i++) {
                    minBounds[i] = glm::min(minBounds[i], v[i]);
                    maxBounds[i] = glm::max(maxBounds[i], v[i]);
                }
            }
        }

        /** Create bounding box Min=-vec, Max=vec */
        AABB(const glm::vec3 &vec) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = -vec[i];
                maxBounds[i] = vec[i];
            }
        }

        /** Create bounding box of sphere */
        AABB(const glm::vec3 &center, float radius) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = center[i] - radius;
                maxBounds[i] = center[i] + radius;
            }
        }

        /** Create bounding box of two arbitrary vectors */
        AABB(const glm::vec3 &vec1, const glm::vec3 &vec2) {
            for (int i = 0; i < 3; i++) {
                minBounds[i] = glm::min(vec1[i], vec2[i]);
                maxBounds[i] = glm::max(vec1[i], vec2[i]);
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

        void getVertices(std::array<glm::vec3, 8> &vertices) {
            vertices[0] = glm::vec3(minBounds[0], minBounds[1], minBounds[2]);
            vertices[1] = glm::vec3(minBounds[0], minBounds[1], maxBounds[2]);
            vertices[2] = glm::vec3(minBounds[0], maxBounds[1], minBounds[2]);
            vertices[3] = glm::vec3(minBounds[0], maxBounds[1], maxBounds[2]);
            vertices[4] = glm::vec3(maxBounds[0], minBounds[1], minBounds[2]);
            vertices[5] = glm::vec3(maxBounds[0], minBounds[1], maxBounds[2]);
            vertices[6] = glm::vec3(maxBounds[0], maxBounds[1], minBounds[2]);
            vertices[7] = glm::vec3(maxBounds[0], maxBounds[1], maxBounds[2]);
        }

        /** Expands box to contain specified point */
        void expandToContain(const glm::vec3& point) {
            minBounds = glm::min(minBounds, point);
            maxBounds = glm::max(maxBounds, point);
        }

        glm::vec3 getCenter() const {
            return (minBounds + maxBounds) * 0.5f;
        }

        glm::vec3 getExtent() const {
            return (maxBounds - minBounds) * 0.5f;
        }

        const glm::vec3 &getMinBounds() const { return minBounds; }
        const glm::vec3 &getMaxBounds() const { return maxBounds; }

    private:
        glm::vec3 minBounds = glm::vec3(0.0f);
        glm::vec3 maxBounds = glm::vec3(0.0f);
    };

}

#endif //IGNIMBRITE_AABB_H