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

    void Camera::setType(Type type) {
        mType = type;
        markDirty();
    }

    void Camera::setPosition(const Vec3f &position) {
        mPosition = position;
        markDirty();
    }

    void Camera::setAspect(float32 aspect) {
        if (std::abs(aspect) <= 0.0005f)
            throw std::runtime_error("Camera aspect near zero");

        mAspect = aspect;
        markDirty();
    }

    void Camera::setFarView(float32 farView) {
        mFarView = farView;
        markDirty();
    }

    void Camera::setNearView(float32 nearView) {
        mNearView = nearView;
        markDirty();
    }

    void Camera::setFov(float32 verticalFovRad) {
        mVerticalFov = verticalFovRad;
        markDirty();
    }

    void Camera::setOrthoWidht(ignimbrite::float32 width) {
        mOrthoWidth = width;
        markDirty();
    }

    void Camera::setClipMatrix(const ignimbrite::Mat4f &clip) {
        mClipMatrix = clip;
        markDirty();
    }

    void Camera::rotate(const Vec3f &axis, float32 angle) {
        mDirection = glm::rotate(mDirection, angle, axis);
        mUp = glm::rotate(mUp, angle, axis);
        markDirty();
    }

    void Camera::setRotation(const Vec3f &axis, float32 angle) {
        mDirection = glm::rotate(glm::vec3(0, 0, 1), angle, axis);
        mUp = glm::rotate(glm::vec3(0, 1, 0), angle, axis);
        markDirty();
    }

    void Camera::move(const Vec3f &vec) {
        mPosition += vec;
        markDirty();
    }

    Camera::Type Camera::getType() const {
        return mType;
    }

    const Vec3f &Camera::getPosition() const {
        return mPosition;
    }

    float32 Camera::getAspect() const {
        return mAspect;
    }

    float32 Camera::getFarClip() const {
        return mFarView;
    }

    float32 Camera::getNearClip() const {
        return mNearView;
    }

    float32 Camera::getFov() const {
        return mVerticalFov;
    }

    float32 Camera::getOrthoWidth() const {
        return mOrthoWidth;
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

    const Mat4f& Camera::getClipMatrix() const {
        return mClipMatrix;
    }

    const Mat4f& Camera::getViewMatrix() const {
        return mViewMatrix;
    }

    const Mat4f& Camera::getProjMatrix() const {
        return mProjMatrix;
    }

    const Mat4f &Camera::getViewProjMatrix() const {
        return mViewProjMatrix;
    }

    void Camera::recalculate() {
        if (!mIsDirty) {
            return;
        }

        mViewMatrix = glm::lookAt(mPosition, mPosition + mDirection, mUp);

//        Depends on rendering device
//        Mat4f clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
//                               0.0f, -1.0f, 0.0f, 0.0f,
//                               0.0f, 0.0f, 0.5f, 0.0f,
//                               0.0f, 0.0f, 0.5f, 1.0f);

        if (isPerspective()) {
            mFrustum.setViewProperties(mDirection, mUp);
            mFrustum.setPosition(mPosition);
            mFrustum.createPerspective(mVerticalFov, mAspect, mNearView, mFarView);

            mProjMatrix = glm::perspective(mVerticalFov, mAspect, mNearView, mFarView);
            mViewProjMatrix = mClipMatrix * mProjMatrix * mViewMatrix;
        }

        if (isOrthographic()) {
            float32 width = mOrthoWidth;
            float32 height = mOrthoWidth / mAspect;
            float32 left = -width / 2;
            float32 right = -left;
            float32 bottom = -height / 2;
            float32 top = -bottom;

            mFrustum.setViewProperties(mDirection, mUp);
            mFrustum.setPosition(mPosition);
            mFrustum.createOrthographic(left, right, bottom, top, mNearView, mFarView);

            mProjMatrix = glm::ortho(left, right, bottom, top, mNearView, mFarView);
            mViewProjMatrix = mClipMatrix * mProjMatrix * mViewMatrix;
        }

        mIsDirty = false;
    }
}