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
        Frustum() :
            mForward(glm::vec3(0,0,1)),
            mUp(glm::vec3(0,1,0)),
            mRight(glm::vec3(1,0,0)) {}

        /** Set vectors to define orientation */
        void setVectors(const glm::vec3 &forward, const glm::vec3 &right, const glm::vec3 &up) {
            mForward = glm::normalize(forward);
            mRight = glm::normalize(right);
            mUp = glm::normalize(up);
        }

        /** Set world space position of this frustum */
        void setPosition(const glm::vec3 &position) {
            mPosition = position;
        }

        const glm::vec3 *getNearVerts() const {
            return mNearVerts;
        }

        const glm::vec3 *getFarVerts() const {
            return mFarVerts;
        }

        /**
         * Calculate planes and near, far vertices for orthographic projection
         * @note to set offset use setPosition(..)
         */
        Frustum &createOrtho(float width, float height, float nearPlane, float farPlane) {
            recalcualte(width / 2.0f, width / 2.0f, height / 2.0f, height / 2.0f, nearPlane, farPlane);
            return *this;
        }

        /**
         * Calculate planes and near, far vertices for perspective projection
         * @param fovRad vertical field of view in radians
         * @param aspect aspect ratio (width/height) of this frustum
         */
        Frustum &createPerspective(float fovRad, float aspect, float nearPlane, float farPlane) {
            float tanfov = tanf(fovRad * 0.5f);

            float nearHHeight = tanfov * nearPlane;
            float nearHWidth = nearHHeight * aspect;

            float farHHeight = tanfov * farPlane;
            float farHWidth = farHHeight * aspect;

            recalcualte(nearHWidth, farHWidth, nearHHeight, farHHeight, nearPlane, farPlane);

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
        void recalcualte(float nearHWidth, float farHWidth,
                float nearHHeight, float farHHeight,
                float nearPlane, float farPlane) {
            mNearVerts[0] = nearHWidth * mRight + nearHHeight * mUp + nearPlane * mForward;
            mNearVerts[1] = -nearHWidth * mRight + nearHHeight * mUp + nearPlane * mForward;
            mNearVerts[2] = -nearHWidth * mRight - nearHHeight * mUp + nearPlane * mForward;
            mNearVerts[3] = nearHWidth * mRight - nearHHeight * mUp + nearPlane * mForward;

            mFarVerts[0] = farHWidth * mRight + farHHeight * mUp + farPlane * mForward;
            mFarVerts[1] = -farHWidth * mRight + farHHeight * mUp + farPlane * mForward;
            mFarVerts[2] = -farHWidth * mRight - farHHeight * mUp + farPlane * mForward;
            mFarVerts[3] = farHWidth * mRight - farHHeight * mUp + farPlane * mForward;

            for (int i = 0; i < 4; i++) {
                mNearVerts[i] += mPosition;
                mFarVerts[i] += mPosition;
            }

            planes[(int)FrustumPlane::Near]     = Plane(mNearVerts[0], mNearVerts[1],mNearVerts[2]);
            planes[(int)FrustumPlane::Far]      = Plane(mFarVerts[2], mFarVerts[1],mFarVerts[0]);

            planes[(int)FrustumPlane::Top]      = Plane(mNearVerts[1], mNearVerts[0], mFarVerts[0]);
            planes[(int)FrustumPlane::Bottom]   = Plane(mNearVerts[3], mNearVerts[2], mFarVerts[2]);

            planes[(int)FrustumPlane::Left]     = Plane(mNearVerts[2], mNearVerts[1], mFarVerts[1]);
            planes[(int)FrustumPlane::Right]    = Plane(mFarVerts[3], mFarVerts[0], mNearVerts[0]);
        }

    private:

        struct Plane {
            glm::vec3   normal;
            float       d;

            Plane() : normal(0,1,0), d(0) {}

            /** Create plane from 3 points in counter clockwise order*/
            Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
                normal = glm::normalize(glm::cross(p3 - p2, p2 - p1));
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

        enum class FrustumPlane {
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

        glm::vec3 mForward;
        glm::vec3 mRight;
        glm::vec3 mUp;

        glm::vec3 mPosition;
    };

}

#endif //IGNIMBRITE_FRUSTUM_H
