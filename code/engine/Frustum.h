/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_FRUSTUM_H
#define IGNIMBRITE_FRUSTUM_H

#include <IncludeStd.h>
#include <AABB.h>

namespace ignimbrite {

    /**
     * @brief View frustum space
     *
     * Frustum class represents view volume of some virtual camera in 3D space.
     * Allows to construct perspective frustum of ortho frustum (as a special case).
     *
     * Frustum class uses right-handed coordinate system, since it meets the
     * convention of world-space 3D transformation for graphics API.
     *
     *          y+
     *          |
     *          |
     *          |
     *          |_______x+
     *         /
     *        /
     *       /
     *      z+
     */
    class Frustum {
    public:

        /** Set vectors to define orientation */
        void setViewProperties(const glm::vec3 &forward, const glm::vec3 &up) {
            mUp = glm::normalize(up);
            mForward = glm::normalize(forward);
            mRight = glm::cross(mForward, mUp);
        }

        void setPosition(const glm::vec3& position) {
            mPosition = position;
        }

        /**
         * Calculate planes and near, far vertices for orthographic projection
         * @note to set offset use setPosition(..)
         */
        void createOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 nearPlane, float32 farPlane) {
            mNearVertices[VertexIndex::UpperRight] = right * mRight + top * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::UpperLeft] = left * mRight + top * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::LowerLeft] = left * mRight + bottom * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::LowerRight] = right * mRight + bottom * mUp + nearPlane * mForward;

            mFarVertices[VertexIndex::UpperRight] = right * mRight + top * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::UpperLeft] = left * mRight + top * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::LowerLeft] = left * mRight + bottom * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::LowerRight] = right * mRight + bottom * mUp + farPlane * mForward;

            for (uint32 i = 0; i < 4; i++) {
                mNearVertices[i] += mPosition;
                mFarVertices[i] += mPosition;
            }

            recalculatePlanes();
        }

        /**
         * Calculate planes and near, far vertices for perspective projection
         * @param fovRad vertical field of view in radians
         * @param aspect aspect ratio (width/height) of this frustum
         */
        void createPerspective(float32 fovRad, float32 aspect, float32 nearPlane, float32 farPlane) {
            float32 tanfov = tanf(fovRad * 0.5f);

            float32 nearHeight = tanfov * nearPlane;
            float32 nearWidth = nearHeight * aspect;

            float32 farHeight = tanfov * farPlane;
            float32 farWidth = farHeight * aspect;

            mNearVertices[VertexIndex::UpperRight] = nearWidth * mRight + nearHeight * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::UpperLeft] = -nearWidth * mRight + nearHeight * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::LowerLeft] = -nearWidth * mRight - nearHeight * mUp + nearPlane * mForward;
            mNearVertices[VertexIndex::LowerRight] = nearWidth * mRight - nearHeight * mUp + nearPlane * mForward;

            mFarVertices[VertexIndex::UpperRight] = farWidth * mRight + farHeight * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::UpperLeft] = -farWidth * mRight + farHeight * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::LowerLeft] = -farWidth * mRight - farHeight * mUp + farPlane * mForward;
            mFarVertices[VertexIndex::LowerRight] = farWidth * mRight - farHeight * mUp + farPlane * mForward;

            for (uint32 i = 0; i < 4; i++) {
                mNearVertices[i] += mPosition;
                mFarVertices[i] += mPosition;
            }

            recalculatePlanes();
        }

        /**
         * Cut frustum for some percent factor.
         * Recalculates frustum with new distance between near and far planes,
         * where new distance equals previous distance between near and far planes scaled by percentage
         * @param percentage Factor to make initial frustum shorter
         */
        void cutFrustum(float32 percentage) {
            percentage = glm::clamp(percentage, 0.0f, 1.0f);

            for (uint32 i = 0; i < mNearVertices.size(); i++) {
                auto delta = mFarVertices[i] - mNearVertices[i];
                delta *= percentage;
                mFarVertices[i] = mNearVertices[i] + delta;
            }

            recalculateFarPlane();
        }

        /** Does this frustum contain or intersect specified AABB? */
        bool isInside(const AABB &aabb) const {
            for (const auto &p: planes) {
                if (!p.onPositiveSideOrIntersects(aabb)) {
                    return false;
                }
            }

            return true;
        }

        const glm::vec3 &getUp() const { return mUp; }
        const glm::vec3 &getRight() const { return mRight; }
        const glm::vec3 &getForward() const { return mForward; }
        const glm::vec3 &getPosition() const { return mPosition; }

        const std::array<glm::vec3,4> &getNearVertices() const { return mNearVertices; }
        const std::array<glm::vec3,4> &getFarVertices() const { return mFarVertices; }

    private:

        void recalculatePlanes() {
            planes[PlaneIndex::Near] = Plane(
                    mNearVertices[VertexIndex::UpperRight],
                    mNearVertices[VertexIndex::UpperLeft],
                    mNearVertices[VertexIndex::LowerLeft]
            );

            planes[PlaneIndex::Far] = Plane(
                    mFarVertices[VertexIndex::LowerLeft],
                    mFarVertices[VertexIndex::UpperLeft],
                    mFarVertices[VertexIndex::UpperRight]
            );

            planes[PlaneIndex::Top] = Plane(
                    mNearVertices[VertexIndex::UpperLeft],
                    mNearVertices[VertexIndex::UpperRight],
                    mFarVertices[VertexIndex::UpperRight]
            );

            planes[PlaneIndex::Bottom] = Plane(
                    mNearVertices[VertexIndex::LowerRight],
                    mNearVertices[VertexIndex::LowerLeft],
                    mFarVertices[VertexIndex::LowerLeft]
            );

            planes[PlaneIndex::Left] = Plane(
                    mNearVertices[VertexIndex::LowerLeft],
                    mNearVertices[VertexIndex::UpperLeft],
                    mFarVertices[VertexIndex::UpperLeft]
            );

            planes[PlaneIndex::Right] = Plane(
                    mFarVertices[VertexIndex::LowerRight],
                    mFarVertices[VertexIndex::UpperRight],
                    mNearVertices[VertexIndex::UpperRight]
            );
        }

        void recalculateFarPlane() {
            planes[PlaneIndex::Far] = Plane(
                    mFarVertices[VertexIndex::LowerLeft],
                    mFarVertices[VertexIndex::UpperLeft],
                    mFarVertices[VertexIndex::UpperRight]
            );
        }

    private:

        struct Plane {
            glm::vec3 normal = glm::vec3(0,1,0);
            float32 d = 0;

            Plane() = default;

            /** Create plane from 3 points in counter clockwise order*/
            Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
                normal = glm::normalize(glm::cross(p3 - p2, p2 - p1));
                d = -glm::dot(normal, p1);
            }

            float32 planeDot(const glm::vec3 &point) const {
                return glm::dot(normal, point) + d;
            }

            /** @return True, if box at least on positive plane side or intersects it */
            bool onPositiveSideOrIntersects(const AABB &aabb) const {
                auto c = aabb.getCenter();
                auto e = aabb.getExtent();
                const auto &n = normal;

                float32 r = glm::dot(e, glm::abs(n));
                float32 s = planeDot(c);

                return s >= -r;
            }
        };

        enum PlaneIndex : uint32 {
            Near = 0,
            Far,
            Left,
            Right,
            Top,
            Bottom
        };

        enum VertexIndex : uint32 {
            UpperRight = 0,
            UpperLeft,
            LowerLeft,
            LowerRight
        };

        /** Frustum planes with normals pointing to the outside of frustum */
        std::array<Plane,6> planes = {};
        /** Near vertices of this frustum in counter clockwise order (First vertex is upper right) */
        std::array<glm::vec3, 4> mNearVertices = {};
        /** Far vertices of this frustum in counter clockwise order (First vertex is upper right) */
        std::array<glm::vec3, 4> mFarVertices = {};

        glm::vec3 mForward  = glm::vec3(0,0,-1);
        glm::vec3 mRight    = glm::vec3(1,0,0);
        glm::vec3 mUp       = glm::vec3(0,1,0);
        glm::vec3 mPosition = glm::vec3(0,0,0);
    };

}

#endif //IGNIMBRITE_FRUSTUM_H