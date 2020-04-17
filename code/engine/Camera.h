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

    class Camera {
    public:

        enum class Type {
            Perspective,
            Orthographic
        };

        Camera() = default;

        void setType(Type type);
        void setPosition(const Vec3f &position);
        /** Set aspect ratio (width / height) */
        void setAspect(float32 aspect);
        void setFarView(float32 farView);
        void setNearView(float32 nearView);
        void setFov(float32 verticalFovRad);
        void setOrthoWidht(float32 width);
        void setClipMatrix(const Mat4f& clip);

        void setRotation(const Vec3f &axis, float32 angle);
        void rotate(const Vec3f &axis, float32 angle);
        void move(const Vec3f &vec);

        /** Aspect ratio (width / height) for this camera */
        Type getType() const;
        float32 getAspect() const;
        float32 getFarClip() const;
        float32 getNearClip() const;
        /** Vertical field of view in degrees for perspective projection */
        float32 getFov() const;
        float32 getOrthoWidth() const;

        const Vec3f &getPosition() const;
        const Vec3f &getDirection() const;
        const Vec3f &getUp() const;
        Vec3f getRight() const;
        const Frustum &getFrustum() const;
        const Mat4f &getClipMatrix() const;
        const Mat4f &getViewMatrix() const;
        const Mat4f &getProjMatrix() const;
        const Mat4f &getViewProjClipMatrix() const;

        void recalculate();
        void markDirty() { mIsDirty = true; }
        bool isDirty() const { return mIsDirty; }
        bool isPerspective() const { return mType == Type::Perspective; }
        bool isOrthographic() const { return mType == Type::Orthographic; }

    private:

        bool mIsDirty = true;
        Type mType = Type::Perspective;

        Vec3f mPosition  = Vec3f(0, 0, 0);
        Vec3f mDirection = Vec3f(0, 0, 1);
        Vec3f mUp        = Vec3f(0, 1, 0);

        /** Aspect ratio (width / height) for this camera */
        float32 mAspect      = 16.0f / 9.0f;
        float32 mFarView     = 100.0f;
        float32 mNearView    = 0.1f;
        /** Vertical field of view in radians for perspective projection */
        float32 mVerticalFov = glm::radians(90.0f);
        float32 mOrthoWidth  = 100.0f;

        Frustum mFrustum;
        Mat4f mClipMatrix         = Mat4f(1.0f);
        Mat4f mViewMatrix         = Mat4f(1.0f);
        Mat4f mProjMatrix         = Mat4f(1.0f);
        Mat4f mViewProjClipMatrix = Mat4f(1.0f);

    };

}

#endif //IGNIMBRITE_CAMERA_H
