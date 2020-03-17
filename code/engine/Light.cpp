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

    Light::Light(LightType ltype) : type(ltype) {}

    const glm::vec3 &Light::getPosition() const {
        return position;
    }

    const glm::vec3 &Light::getDirection() const {
        return direction;
    }

    const glm::vec3 &Light::getUp() const {
        return up;
    }

    const glm::vec3 &Light::getColor() const {
        return color;
    }

    float Light::getIntensity() const {
        return intensity;
    }

    float Light::getRange() const {
        return range;
    }

    float Light::getSpotAngle() const {
        return spotAngle;
    }

    LightType Light::getType() const {
        return type;
    }

    float Light::isShadowCast() const {
        return castShadow;
    }

    float Light::getShadowBias() const {
        return shadowBias;
    }

    float Light::getShadowNormalBias() const {
        return shadowBias;
    }

    float Light::getShadowNearPlane() const {
        return shadowNearPlane;
    }

    const Frustum &Light::getFrustum() const {
        if (type != LightType::Directional && type != LightType::Spot) {
            throw std::runtime_error("Frustum is used only by directional and spot light");
        }

        return frustum;
    }

    const AABB &Light::getAABB() const {
        if (type != LightType::Point) {
            throw std::runtime_error("AABB is used only by point light");
        }

        return aabb;
    }

    void Light::setPosition(const glm::vec3 &position) {
        Light::position = position;

        if (type == LightType::Spot) {
            rebuildSpotFrustum();
        } else if (type == LightType::Point) {
            rebuildPointAABB();
        }
    }

    void Light::setColor(const glm::vec3 &color) {
        Light::color = color;
    }

    void Light::setIntensity(float intensity) {
        Light::intensity = intensity;
    }

    void Light::setRange(float range) {
        Light::range = range;

        if (type == LightType::Spot) {
            rebuildSpotFrustum();
        } else if (type == LightType::Point) {
            rebuildPointAABB();
        }
    }

    void Light::setSpotAngle(float spotAngle) {
        Light::spotAngle = spotAngle;

        if (type == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }

    void Light::setCastShadow(bool castShadow) {
        Light::castShadow = castShadow;
    }

    void Light::setShadowBias(float shadowBias) {
        Light::shadowBias = shadowBias;
    }

    void Light::setShadowNormalBias(float shadowNormalBias) {
        Light::shadowNormalBias = shadowNormalBias;
    }

    void Light::setShadowNearPlane(float shadowNearPlane) {
        Light::shadowNearPlane = shadowNearPlane;

        if (type == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }

    void Light::setType(LightType type) {

        LightType oldType = Light::type;
        Light::type = type;

        if (oldType != type) {
            if (type == LightType::Point) {
                rebuildPointAABB();
            } else if (type == LightType::Spot) {
                rebuildSpotFrustum();
            }
        }
    }

    void Light::rebuildSpotFrustum() {
        if (type != LightType::Spot) {
            return;
        }

        frustum.setViewProperties(position, getDirection(), getUp());
        frustum.createPerspective(spotAngle, 1, 0.0001f, range);
    }

    void Light::rebuildPointAABB() {
        if (type != LightType::Point) {
            return;
        }

        aabb = AABB(position, range);
    }

    void Light::fitCameraFrustum(const Frustum &cameraFrustum) {
        if (type != LightType::Directional) {
            throw std::runtime_error("Fitting frustum is allowed only for directional lights");
        }

        // transform all vertices from world to light's space
        glm::mat4 lightSpace = glm::lookAt(glm::vec3(0, 0, 0), getDirection(), getUp());

        const glm::vec3 *cnearVerts = cameraFrustum.getNearVertices();
        const glm::vec3 *cfarVerts = cameraFrustum.getFarVertices();
        glm::vec4 lnearVerts[4];
        glm::vec4 lfarVerts[4];

        for (uint32 i = 0; i < 4; i++) {
            lnearVerts[i] = lightSpace * glm::vec4(cnearVerts[i], 1.0f);
            lfarVerts[i] = lightSpace * glm::vec4(cfarVerts[i], 1.0f);
        }

        // get bounding box, in light space
        glm::vec3 bmin = glm::vec3(lnearVerts[0]);
        glm::vec3 bmax = glm::vec3(lnearVerts[0]);

        for (uint32 j = 0; j < 3; j++) {
            for (uint32 i = 0; i < 4; i++) {
                bmin[j] = std::min(bmin[j], lnearVerts[i][j]);
                bmin[j] = std::min(bmin[j], lfarVerts[i][j]);
                bmax[j] = std::max(bmax[j], lnearVerts[i][j]);
                bmax[j] = std::max(bmax[j], lfarVerts[i][j]);
            }
        }

        glm::vec3 center = (bmax + bmin) / 2.0f;
        center = glm::vec3(glm::inverse(lightSpace) * glm::vec4(center, 1.0f));

        float width = bmax[0] - bmin[0];
        float height = bmax[1] - bmin[1];

        frustum.setViewProperties(center, getDirection(), getUp());
        frustum.createOrthographic(width, height, bmin[2], bmax[2]);
    }

    void Light::setDirection(const glm::vec3 &direction, const glm::vec3 &up) {
        Light::direction = direction;
        Light::up = up;

        if (type == LightType::Spot) {
            rebuildSpotFrustum();
        }
    }
}