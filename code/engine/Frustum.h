/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_FRUSTUM_H
#define IGNIMBRITE_FRUSTUM_H

#include <AABB.h>

namespace ignimbrite
{

    class Frustum
    {
    public:
        /**
         * Set field of view of this frustum
         * @param fovy vertical fov in degrees
         */
        Frustum &setFov(float fovy)
        {
            mFovy = fovy;
            return *this;
        }

        /**
         * Set aspect ratio (width/height) of this frustum
         * @param aspect width/height
         */
        Frustum &setAspect(float aspect)
        {
            mAspect = aspect;
            return *this;
        }

        Frustum &setNear(float znear)
        {
            mNear = znear;
            return *this;
        }

        Frustum &setFar(float zfar)
        {
            mFar = zfar;
            return *this;
        }

        Frustum &setTransform(const glm::mat4x4 &transform) {
            mTransform = transform;
            return *this;
        }

        /**
         * Calculate planes and near, far vertices
         */
        Frustum &build() {
            
            float fovRad = mFovy * M_PI / 180.0f;
            float tanfov = tanf(fovRad * 0.5f);

            float nearHHeight = tanfov * mNear;
            float nearHWidth = nearHHeight * mAspect;

            float farHHeight = tanfov * mFar;
            float farHWidth = farHHeight * mAspect;

            // calculate vertices of frustum in local space
            glm::vec4 localNearVerts[4];
            glm::vec4 localFarVerts[4];

            localNearVerts[0] = glm::vec4(nearHWidth, nearHHeight, mNear, 0.0f);
            localNearVerts[1] = glm::vec4(-nearHWidth, nearHHeight, mNear, 0.0f);
            localNearVerts[2] = glm::vec4(-nearHWidth, -nearHHeight, mNear, 0.0f);
            localNearVerts[3] = glm::vec4(nearHWidth, -nearHHeight, mNear, 0.0f);

            localFarVerts[0] = glm::vec4(farHWidth, farHHeight, mFar, 0.0f);
            localFarVerts[1] = glm::vec4(-farHWidth, farHHeight, mFar, 0.0f);
            localFarVerts[2] = glm::vec4(-farHWidth, -farHHeight, mFar, 0.0f);
            localFarVerts[3] = glm::vec4(farHWidth, -farHHeight, mFar, 0.0f);

            // transform to world space
            for (int i = 0; i < 4; i++) {
                mNearVerts[i] = glm::vec3(mTransform * localNearVerts[i]);
            }

            for (int i = 0; i < 4; i++) {
                mFarVerts[i] = glm::vec3(mTransform * localFarVerts[i]);
            }

            planes[(int)FrustumPlane::Near].fromPoints(mNearVerts[0], mNearVerts[1],mNearVerts[2]);
            planes[(int)FrustumPlane::Far].fromPoints(mFarVerts[2], mFarVerts[1],mFarVerts[0]);

            planes[(int)FrustumPlane::Top].fromPoints(mNearVerts[1], mNearVerts[0], mFarVerts[0]);
            planes[(int)FrustumPlane::Bottom].fromPoints(mNearVerts[3], mNearVerts[2], mFarVerts[2]);

            planes[(int)FrustumPlane::Left].fromPoints(mNearVerts[2], mNearVerts[1], mFarVerts[1]);
            planes[(int)FrustumPlane::Right].fromPoints(mFarVerts[3], mFarVerts[0], mNearVerts[0]);

            return *this;
        }

        /**
         * Does this frustum contain or intersect specified AABB?
         */
        bool isInside(const AABB &aabb) const {
            for (int i = 0; i < 6; i++) {
                // if AABB doesn't intersect with negative halfspace of each plane,
                // then it's not inside frustum
                if (!planes[i].intersect(aabb)) {
                    return false;
                }
            }

            return true;
        }

    private:

        struct Plane {
            glm::vec3   normal;
            float       d;

            /** Create plane from 3 points in counter clockwise order*/
            void fromPoints(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
                normal = glm::cross(p3 - p2, p2 - p1);
                normal = glm::normalize(normal);
                d = -glm::dot(normal, p1);
            }

            float planeDot(const glm::vec3 &point) const {
                return glm::dot(normal, point) + d;
            }

            /** Does specified AABB intersect with negative halfspace of this plane? */
            bool intersect(const AABB &aabb) const {
                const glm::vec3 &c = aabb.getCenter();
                const glm::vec3 &e = aabb.getExtent();
                const glm::vec3 &n = normal;

                // get projection of AABB's extent on plane's normal
                float r = e[0] * std::abs(n[0]) + e[1] * glm::abs(n[1]) + e[2] * glm::abs(n[2]);
                // get distance of AABB's center from plane
                float s = planeDot(c);

                // intersection with negative halfspace occurs
                // if s is in (-inf, +r]
                return s <= r;
            }
        };

        enum class FrustumPlane
        {
            Near, Far,
            Left, Right,
            Top, Bottom
        };

        /** Frustum planes with normals pointing to the outside of frustum */
        Plane planes[6];

        /**
         * Near vertices of this frustum in counter clockwise order.
         * First vertex is upper right*/
        glm::vec3 mNearVerts[4];

        /**
         * Far vertices of this frustum in counter clockwise order.
         * First vertex is upper right*/
        glm::vec3 mFarVerts[4];

        /** Transformation matrix of this frustum */
        glm::mat4x4 mTransform;

        float mFovy;
        float mAspect;
        float mNear;
        float mFar;
    };

}

#endif //IGNIMBRITE_FRUSTUM_H
