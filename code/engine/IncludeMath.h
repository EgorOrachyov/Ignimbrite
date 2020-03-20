/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_INCLUDEMATH_H
#define IGNIMBRITE_INCLUDEMATH_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <AABB.h>

namespace ignimbrite {
    /** Place type aliases here */
    using Vec2f = glm::vec2;
    using Vec3f = glm::vec3;
    using Vec4f = glm::vec4;
    using Mat4f = glm::mat4x4;
}

#endif //IGNIMBRITE_INCLUDEMATH_H
