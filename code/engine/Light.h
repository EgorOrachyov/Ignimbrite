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

        const glm::vec3 &getPosition() const;
        const glm::vec3 &getDirection() const;
        const glm::vec3 &getUp() const;
        const glm::vec3 &getColor() const;
        float getIntensity() const;
        float getRange() const;
        float getSpotAngle() const;
        float isShadowCast() const;
        float getShadowBias() const;
        float getShadowNormalBias() const;
        float getShadowNearPlane() const;
        /**
         * Get frustum for directional or spot light
         */
        const Frustum &getFrustum() const;
        /**
         * Get AABB for point light
         */
        const AABB &getAABB() const;
        LightType getType() const;
        void setType(LightType type);

        void setPosition(const glm::vec3 &position);
        void setColor(const glm::vec3 &color);
        void setIntensity(float intensity);
        void setRange(float range);
        /** Get fov angle in radians of spot light */
        void setSpotAngle(float spotAngle);
        void setCastShadow(bool castShadow);
        void setShadowBias(float shadowBias);
        void setShadowNormalBias(float shadowNormalBias);
        void setShadowNearPlane(float shadowNearPlane);

        void setDirection(const glm::vec3 &direction, const glm::vec3 &up);

        /**
         * Recalculate directional light's frustum to fit camera's view
         * @note only for directional light
         */
        void fitCameraFrustum(const Frustum &cameraFrustum);

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
        LightType type;

        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 up;
        glm::vec3 color;
        float intensity;

        float range;
        float spotAngle;

        bool castShadow;
        float shadowBias;
        float shadowNormalBias;
        float shadowNearPlane;

        /** AABB for point light */
        AABB aabb;
        /** Frustum for directional and spot light */
        Frustum frustum;
    };

}

#endif //IGNIMBRITE_LIGHT_H
