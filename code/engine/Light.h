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

#include <Frustum.h>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace ignimbrite {

    enum class LightType {
        Directional,
        Ambient,
        Point,
        Spot
    };

    class Light {
    public:
        Light();
        Light(LightType type);

        LightType getType() const;
        const glm::vec3 &getPosition() const;
        const glm::vec3 &getColor() const;
        const glm::vec3 &getDirection() const;
        const glm::vec3 &getUp() const;
        float32 getIntensity() const;
        float32 getRange() const;
        /** Get fov angle in degrees of spot light */
        float32 getSpotAngle() const;
        bool isShadowCast() const;
        float32 getShadowBias() const;
        float32 getShadowNormalBias() const;
        float32 getShadowNearPlane() const;
        /** Get frustum for directional or spot light */
        const Frustum &getFrustum() const;
        /** Get AABB for point light */
        const AABB &getAABB() const;

        void setType(LightType type);
        void setPosition(const glm::vec3 &position);
        void setColor(const glm::vec3 &color);
        void setDirection(const glm::vec3 &direction, const glm::vec3 &up);
        void setIntensity(float32 intensity);
        void setRange(float32 range);
        /** Set fov angle for spot light in degrees */
        void setSpotAngle(float32 spotAngleDeg);
        void setCastShadow(bool castShadow);
        void setShadowBias(float32 shadowBias);
        void setShadowNormalBias(float32 shadowNormalBias);
        void setShadowNearPlane(float32 shadowNearPlane);

        /**
         * Recalculate directional light's frustum to fit camera's view
         * @param percentage fit only a part of camera's frustum.
         *              For instance, 0.25 means that light's frustum will only
         *              contain a quarter of camera's frustum
         * @note only for directional light
         */
        void fitCameraFrustum(const Frustum &cameraFrustum, float32 percentage = 1.0f);

        /**
         * Get View-Projection matrices for this light.
         * @param outMatrices an output vector with required matrices.
         * The length will be 1 for directional and spot lights, and 6 for point lights.
         * For point lights the order is (+X, -X, +Y, -Y, +Z, -Z)
         */
        void getLightSpace(std::vector<glm::mat4> &outMatrices) const;

    private:
        /**
         * Rebuild frustum for spot light.
         * @note for directional lights, use fitCameraFrustum(..)
         */
        void rebuildSpotFrustum();
        /**
         * Rebuild frustum for point light.
         */
        void rebuildPointAABB();

    private:
        LightType mType;

        glm::vec3 mPosition;
        glm::vec3 mDirection;
        glm::vec3 mUp;
        glm::vec3 mColor;
        float32 mIntensity;

        float32 mRange;
        float32 mSpotAngle;
        /** Near plane for spot and point lights */
        constexpr const static float32 mLightPerspectiveNear = 0.0001f;

        bool mCastShadow;
        float32 mShadowBias;
        float32 mShadowNormalBias;
        float32 mShadowNearPlane;

        /** AABB for point light */
        AABB mAabb;
        /** Frustum for directional and spot light */
        Frustum mFrustum;
        /** View-projection matrix for directional and spot lights */
        glm::mat4 mViewProjMatrix;
    };

}

#endif //IGNIMBRITE_LIGHT_H
