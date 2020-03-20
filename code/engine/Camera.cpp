/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/

#include "Camera.h"

namespace ignimbrite {

    float32 Camera::getSize() const {
        return mSize;
    }

    CameraType Camera::getType() const {
        return mType;
    }

    const Vec3f &Camera::getPosition() const {
        return mPosition;
    }

    float32 Camera::getAspect() const {
        return mAspect;
    }

    float32 Camera::getFarClip() const {
        return mFarClip;
    }

    float32 Camera::getNearClip() const {
        return mNearClip;
    }

    float32 Camera::getFov() const {
        return mVerticalFov;
    }

    void Camera::setType(CameraType type) {
        mType = type;
        mIsDirty = true;
    }

    void Camera::setPosition(const Vec3f &position) {
        mPosition = position;
        mIsDirty = true;
    }

    void Camera::setAspect(float32 aspect) {
        mAspect = aspect;
        mIsDirty = true;
    }

    void Camera::setFarClip(float32 farClip) {
        mFarClip = farClip;
        mIsDirty = true;
    }

    void Camera::setNearClip(float32 nearClip) {
        mNearClip = nearClip;
        mIsDirty = true;
    }

    void Camera::setFov(float32 verticalFov) {
        mVerticalFov = verticalFov;
        mIsDirty = true;
    }

    void Camera::setSize(float32 size) {
        mSize = size;
        mIsDirty = true;
    }

    void Camera::rotate(const Vec3f &axis, float32 angle) {
        mDirection = glm::normalize(glm::rotate(mDirection, angle, axis));
        mUp =  glm::normalize(glm::rotate(mUp, angle, axis));

        mIsDirty = true;
    }

    void Camera::setRotation(const Vec3f &axis, float32 angle) {
        mDirection = glm::rotate(glm::vec3(0, 0, 1), angle, axis);
        mUp = glm::rotate(glm::vec3(0, 1, 0), angle, axis);

        mIsDirty = true;
    }

    void Camera::move(const Vec3f &vec) {
        mPosition += vec;
        mIsDirty = true;
    }

    const Vec3f &Camera::getDirection() const {
        return mDirection;
    }

    const Vec3f &Camera::getUp() const {
        return mUp;
    }

    Vec3f Camera::getRight() const {
        return glm::cross(getDirection(), getUp());
    }

    const Frustum &Camera::getFrustum() const {
        return mFrustum;
    }

    const Mat4f &Camera::getViewProjMatrix() const {
        return mViewProjMatrix;
    }

    void Camera::recalculate() {
        if (!mIsDirty) {
            return;
        }

        Mat4f view = glm::lookAt(mPosition, mPosition + mDirection, mUp);
        Mat4f clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, -1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.5f, 0.0f,
                              0.0f, 0.0f, 0.5f, 1.0f);

        if (mType == CameraType::Perspective) {
            float fovyRad = glm::radians(mVerticalFov);

            mFrustum.setViewProperties(mDirection, mUp);
            mFrustum.createPerspective(mPosition, fovyRad, mAspect, mNearClip, mFarClip);

            Mat4f proj = glm::perspective(fovyRad, mAspect, mNearClip, mFarClip);
            mViewProjMatrix = clip * proj * view;

        } else {
            float width = mSize;
            float height = mSize / mAspect;

            Mat4f proj = glm::ortho(-width / 2, width / 2, -height / 2, height / 2, mNearClip, mFarClip);
            mViewProjMatrix = clip * proj * view;
        }

        mIsDirty = false;
    }
}