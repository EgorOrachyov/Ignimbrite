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
        // Create bounding box Min=-vec, Max=vec
        AABB(const glm::vec3 &vec);
        // Create bounding box of sphere
        AABB(const glm::vec3 &center, const float radius);
        // Create bounding box of two arbitary vectors
        AABB(const glm::vec3 &vec1, const glm::vec3 &vec2);

        // Does this AABB contain other one?
        bool Contains(const AABB &aabb) const;
        // Does this AABB contain a point?
        bool Contains(const glm::vec3 &point) const;

        // Is there intersection between this and other AABB?
        bool Intersect(const AABB &other);

        // TODO: plane intersections

    private:
        glm::vec3 minBounds;
        glm::vec3 maxBounds;
    };

}

#endif //IGNIMBRITE_AABB_H