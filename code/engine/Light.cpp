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
#include <glm/gtc/matrix_transform.hpp>
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

    bool Light::isShadowCast() const {
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
        mRange = range > 0.0f ? range : 0.0f;

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        } else if (mType == LightType::Point) {
            rebuildPointAABB();
        }
    }

    void Light::setSpotAngle(float32 spotAngle) {
        mSpotAngle = glm::clamp(spotAngle, 0.0f, 180.0f - glm::epsilon<float>());

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
        mFrustum.createPerspective(getPosition(), glm::radians(mSpotAngle), 1, mLightPerspectiveNear, mRange);

        const glm::mat4 &view = glm::lookAt(getPosition(), getPosition() + getDirection(), getUp());
        const glm::mat4 &proj = glm::perspective(glm::radians(mSpotAngle), 1.0f, mLightPerspectiveNear, mRange);

        mViewProjMatrix = proj * view;
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

        mPosition = cameraFrustum.getPosition();

        const glm::vec3 *cnearVerts = cameraFrustum.getNearVertices();
        const glm::vec3 *cfarVerts = cameraFrustum.getFarVertices();
        glm::vec3 lVerts[8];

        for (uint32 i = 0; i < 4; i++) {
            lVerts[i] = cnearVerts[i];
            lVerts[i + 4] = cfarVerts[i];
        }

        const glm::vec3 &d = getDirection();
        const glm::vec3 &u = getUp();
        const glm::vec3 &r = glm::cross(d, u);

        float32 left = glm::dot(lVerts[0], r);
        float32 bottom = glm::dot(lVerts[0], u);
        float32 nearPlane = glm::dot(lVerts[0], d);
        float32 right = left;
        float32 top = bottom;
        float32 farPlane = nearPlane;

        for (const glm::vec3 &lVert : lVerts) {
            left = std::min(left, glm::dot(lVert, r));
            right = std::max(right, glm::dot(lVert, r));

            bottom = std::min(bottom, glm::dot(lVert, u));
            top = std::max(top, glm::dot(lVert, u));

            nearPlane = std::min(nearPlane, glm::dot(lVert, d));
            farPlane = std::max(farPlane, glm::dot(lVert, d));
        }

        mFrustum.setViewProperties(getDirection(), getUp());
        mFrustum.createOrthographic(left, right, bottom, top, nearPlane, farPlane);

        const glm::mat4 &view = glm::lookAt(glm::vec3(0, 0, 0), getDirection(), getUp());
        const glm::mat4 &proj = glm::ortho(left, right, bottom, top, nearPlane, farPlane);

        mViewProjMatrix = proj * view;
    }

    void Light::setDirection(const glm::vec3 &direction, const glm::vec3 &up) {
        mDirection = glm::normalize(direction);
        mUp = glm::normalize(up);

        if (mType == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }

    void Light::getLightSpace(std::vector<glm::mat4> &outMatrices) const {
        if (mType == LightType::Spot) {
            outMatrices.resize(1);
            outMatrices[0] = mViewProjMatrix;
        } else if (mType == LightType::Directional) {
            outMatrices.resize(1);
            outMatrices[0] = mViewProjMatrix;
        } else if (mType == LightType::Point) {
            outMatrices.resize(6);

            // directions and up vectors
            glm::vec3 curDir[6][2] = {
                    {glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)},
                    {glm::vec3(1, 0, 0),  glm::vec3(0, 1, 0)},
                    {glm::vec3(0, 1, 0),  glm::vec3(0, 0, 1)},
                    {glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)},
                    {glm::vec3(0, 0, 1),  glm::vec3(0, 1, 0)},
                    {glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)}
            };

            const glm::mat4 &proj = glm::perspective(glm::pi<float32>() / 2.0f, 1.0f, mLightPerspectiveNear, mRange);

            for (int i = 0; i < 6; i++) {
                const glm::mat4 &view = glm::lookAt(getPosition(), getPosition() + curDir[i][0], curDir[i][1]);
                outMatrices[i] = proj * view;
            }
        } else {
            throw std::runtime_error("Light space is only available for directional, spot and point lights");
        }
    }
}