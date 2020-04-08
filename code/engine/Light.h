/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_LIGHT_H
#define IGNIMBRITE_LIGHT_H

#include <IncludeStd.h>
#include <IncludeMath.h>
#include <Frustum.h>

namespace ignimbrite {

    class Light {
    public:

        enum class Type {
            Directional
        };

        Light() = default;

        void setType(Type type);
        void setPosition(const Vec3f &position);
        void setColor(const Vec3f &color);
        void setIntensity(float32 intensity);
        void setCastShadow(bool castShadow);
        void setRotation(const Vec3f& axis, float32 angle);
        void rotate(const Vec3f& axis, float32 angle);
        void move(const Vec3f& vec);
        void setClipMatrix(const Mat4f &clipMatrix) { mClipMatrix = clipMatrix; };

        Type getType() const { return mType; }
        const Vec3f &getPosition() const { return mPosition; }
        const Vec3f &getDirection() const { return mDirection; }
        const Vec3f &getUp() const { return mUp;}
        Vec3f getRight() const { return glm::cross(getDirection(), getUp());}
        const Vec3f &getColor() const { return mColor; }
        float32 getIntensity() const { return mIntensity; }
        bool castShadow() const { return mCastShadow; }
        const Frustum &getFrustum() const { return mFrustum; }

        const Mat4f &getViewMatrix() const { return mViewMatrix; }
        const Mat4f &getProjMatrix() const { return mProjectionMatrix; }
        const Mat4f &getClipMatrix() const { return mClipMatrix; }
        Mat4f getViewProjClipMatrix() const { return mClipMatrix * mProjectionMatrix * mViewMatrix; }

        void buildViewFrustum(const Frustum &cameraFrustum);

    private:

        Type mType = Type::Directional;

        Vec3f mPosition     = Vec3f(0,0,0);
        Vec3f mDirection    = Vec3f(0,0,1);
        Vec3f mUp           = Vec3f(0,1,0);
        Vec3f mColor        = Vec3f(1,1,1);

        float32 mIntensity  = 1.0f;
        bool mCastShadow    = false;

        Frustum mFrustum;

        Mat4f mViewMatrix = Mat4f(1.0f);
        Mat4f mProjectionMatrix = Mat4f(1.0f);
        Mat4f mClipMatrix = Mat4f(1.0f);
    };

}

#endif //IGNIMBRITE_LIGHT_H
