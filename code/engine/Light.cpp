/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stdexcept>
#include "Light.h"

namespace ignimbrite {

    Light::Light() : Light(LightType::Point) {}

    Light::Light(LightType ltype) : mType(ltype) {}

    const glm::vec3 &Light::getPosition() const {
        return mPosition;
    }

    const glm::vec3 &Light::getDirection() const {
        return mDirection;
    }

    const glm::vec3 &Light::getUp() const {
        return mUp;
    }

    const glm::vec3 &Light::getColor() const {
        return mColor;
    }

    float32 Light::getIntensity() const {
        return mIntensity;
    }

    float32 Light::getRange() const {
        return mRange;
    }

    float32 Light::getSpotAngle() const {
        return mSpotAngle;
    }

    LightType Light::getType() const {
        return mType;
    }

    float32 Light::isShadowCast() const {
        return mCastShadow;
    }

    float32 Light::getShadowBias() const {
        return mShadowBias;
    }

    float32 Light::getShadowNormalBias() const {
        return mShadowBias;
    }

    float32 Light::getShadowNearPlane() const {
        return mShadowNearPlane;
    }

    const Frustum &Light::getFrustum() const {
        if (mType != LightType::Directional && mType != LightType::Spot) {
            throw std::runtime_error("Frustum is used only by directional and spot light");
        }

        return mFrustum;
    }

    const AABB &Light::getAABB() const {
        if (mType != LightType::Point) {
            throw std::runtime_error("AABB is used only by point light");
        }

        return mAabb;
    }

    void Light::setPosition(const glm::vec3 &position) {
        mPosition = position;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        } else if (mType == LightType::Point) {
            rebuildPointAABB();
        }
    }

    void Light::setColor(const glm::vec3 &color) {
        mColor = color;
    }

    void Light::setIntensity(float32 intensity) {
        mIntensity = intensity;
    }

    void Light::setRange(float32 range) {
        mRange = range;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        } else if (mType == LightType::Point) {
            rebuildPointAABB();
        }
    }

    void Light::setSpotAngle(float32 spotAngle) {
        mSpotAngle = spotAngle;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }

    void Light::setCastShadow(bool castShadow) {
        mCastShadow = castShadow;
    }

    void Light::setShadowBias(float32 shadowBias) {
        mShadowBias = shadowBias;
    }

    void Light::setShadowNormalBias(float32 shadowNormalBias) {
        mShadowNormalBias = shadowNormalBias;
    }

    void Light::setShadowNearPlane(float32 shadowNearPlane) {
        mShadowNearPlane = shadowNearPlane;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }

    void Light::setType(LightType type) {

        LightType oldType = mType;
        mType = type;

        if (oldType != type) {
            if (type == LightType::Point) {
                rebuildPointAABB();
            } else if (type == LightType::Spot) {
                rebuildSpotFrustum();
            }
        }
    }

    void Light::rebuildSpotFrustum() {
        if (mType != LightType::Spot) {
            return;
        }

        mFrustum.setViewProperties(getDirection(), getUp());
        mFrustum.createPerspective(getPosition(), mSpotAngle, 1, 0.0001f, mRange);
    }

    void Light::rebuildPointAABB() {
        if (mType != LightType::Point) {
            return;
        }

        mAabb = AABB(mPosition, mRange);
    }

    void Light::fitCameraFrustum(const Frustum &cameraFrustum, float32 percentage) {
        if (mType != LightType::Directional) {
            throw std::runtime_error("Fitting frustum is allowed only for directional lights");
        }

        // transform all vertices from world to light's space
        glm::mat4 lightSpace = glm::lookAt(glm::vec3(0, 0, 0), getDirection(), getUp());
        glm::mat4 invLightSpace = glm::inverse(lightSpace);

        const glm::vec3 *cnearVerts = cameraFrustum.getNearVertices();
        const glm::vec3 *cfarVerts = cameraFrustum.getFarVertices();
        glm::vec4 lVerts[8];

        for (uint32 i = 0; i < 4; i++) {
            lVerts[i] = invLightSpace * glm::vec4(cnearVerts[i], 1.0f);
            lVerts[i + 4] = invLightSpace * glm::vec4(cfarVerts[i], 1.0f);
        }

        // get bounding box, in light space
        glm::vec3 bmin = lVerts[0];
        glm::vec3 bmax = lVerts[0];

        for (auto &lVert : lVerts) {
            for (uint32 j = 0; j < 3; j++) {
                bmin[j] = std::min(bmin[j], lVert[j]);
                bmax[j] = std::max(bmax[j], lVert[j]);
            }
        }

        mPosition = lightSpace * glm::vec4((bmax + bmin) / 2.0f, 1.0f);

        float32 nearPlane = bmin[2];
        float32 farPlane = nearPlane + (bmax[2] - nearPlane) * percentage;

        mFrustum.setViewProperties(getDirection(), getUp());
        mFrustum.createOrthographic(-bmin[0], -bmax[0], bmin[1], bmax[1], nearPlane, farPlane);
    }

    void Light::setDirection(const glm::vec3 &direction, const glm::vec3 &up) {
        mDirection = direction;
        mUp = up;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }
}