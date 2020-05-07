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

    void Light::setPosition(const glm::vec3 &position) {
        mPosition = position;
    }

    void Light::setColor(const glm::vec3 &color) {
        mColor = glm::clamp(color, Vec3f(0.0f), Vec3f(1.0f));
    }

    void Light::setIntensity(float32 intensity) {
        mIntensity = intensity;
    }

    void Light::setCastShadow(bool castShadow) {
        mCastShadow = castShadow;
    }

    void Light::rotate(const ignimbrite::Vec3f &axis, ignimbrite::float32 angle) {
        mDirection = glm::rotate(mDirection, angle, axis);
        mUp = glm::rotate(mUp, angle, axis);
    }

    void Light::setRotation(const Vec3f &axis, float32 angle) {
        mDirection = glm::rotate(glm::vec3(0, 0, 1), angle, axis);
        mUp = glm::rotate(glm::vec3(0, 1, 0), angle, axis);
    }

    void Light::move(const ignimbrite::Vec3f &vec) {
        mPosition += vec;
    }

    void Light::setType(Type type) {
        mType = type;
    }

    void Light::buildViewFrustum(const Frustum &cameraFrustum) {
        mPosition = cameraFrustum.getPosition();

        const auto &camNearVerts = cameraFrustum.getNearVertices();
        const auto &camFarVerts = cameraFrustum.getFarVertices();
        glm::vec3 frustumVerts[8];

        for (uint32 i = 0; i < 4; i++) {
            frustumVerts[i] = camNearVerts[i];
            frustumVerts[i + 4] = camFarVerts[i];
        }

        const auto& d = getDirection();
        const auto& u = getUp();
        auto        r = glm::cross(d, u);

        float32 left = glm::dot(frustumVerts[0], r);
        float32 bottom = glm::dot(frustumVerts[0], u);
        float32 nearPlane = glm::dot(frustumVerts[0], d);
        float32 right = left;
        float32 top = bottom;
        float32 farPlane = nearPlane;

        for (const glm::vec3 &lVert : frustumVerts) {
            left = std::min(left, glm::dot(lVert, r));
            right = std::max(right, glm::dot(lVert, r));

            bottom = std::min(bottom, glm::dot(lVert, u));
            top = std::max(top, glm::dot(lVert, u));

            nearPlane = std::min(nearPlane, glm::dot(lVert, d));
            farPlane = std::max(farPlane, glm::dot(lVert, d));
        }

        mFrustum.setViewProperties(getDirection(), getUp());
        mFrustum.createOrthographic(left, right, bottom, top, nearPlane - 20, farPlane);

        mViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), getDirection(), getUp());
        mProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane - 20, farPlane);
    }

}