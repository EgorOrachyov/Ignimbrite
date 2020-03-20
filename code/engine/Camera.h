/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_CAMERA_H
#define IGNIMBRITE_CAMERA_H

#include <IncludeMath.h>
#include <Frustum.h>

namespace ignimbrite {

    enum class CameraType {
        Perspective,
        Orthographic
    };

    class Camera {
    public:
        Camera() = default;

        void setType(CameraType type);
        void setPosition(const Vec3f &position);
        /** Set aspect ratio (width / height) */
        void setAspect(float32 aspect);
        void setFarClip(float32 farClip);
        void setNearClip(float32 nearClip);
        void setFov(float32 verticalFov);
        /** Set width of a box for orthographic projection */
        void setSize(float32 size);

        CameraType getType() const;
        const Vec3f &getPosition() const;
        /** Aspect ratio (width / height) for this camera */
        float32 getAspect() const;
        float32 getFarClip() const;
        float32 getNearClip() const;
        /** Vertical field of view in degrees for perspective projection */
        float32 getFov() const;
        /** Width of a box for orthographic projection */
        float32 getSize() const;

        void setRotation(const Vec3f &axis, float32 angle);
        void rotate(const Vec3f &axis, float32 angle);
        void move(const Vec3f &vec);

        const Vec3f &getDirection() const;
        const Vec3f &getUp() const;
        Vec3f getRight() const;
        const Frustum &getFrustum() const;
        const Mat4f &getViewProjMatrix() const;

        void recalculate();

    private:
        CameraType mType        = CameraType::Perspective;

        Vec3f mPosition         = Vec3f(0, 0, 0);
        Vec3f mDirection        = Vec3f(0, 0, 1);
        Vec3f mUp               = Vec3f(0, 1, 0);

        /** Aspect ratio (width / height) for this camera */
        float32 mAspect         = 16.0f / 9.0f;

        float32 mFarClip        = 100.0f;
        float32 mNearClip       = 0.1f;

        /** Vertical field of view in degrees for perspective projection */
        float32 mVerticalFov    = 90.0f;
        /** Width of a box for orthographic projection */
        float32 mSize           = 10.0f;

        Frustum mFrustum;
        Mat4f mViewProjMatrix   = Mat4f(1.0f);
        bool mIsDirty           = true;
    };

}

#endif //IGNIMBRITE_CAMERA_H
