/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <RenderEngine.h>
#include <PipelineContext.h>

namespace ignimbrite {

    RenderEngine::RenderEngine() {
        mContext = std::make_shared<IRenderContext>();
    }

    void RenderEngine::setCamera(RefCounted<Camera> camera) {
        if (mCamera == camera)
            throw std::runtime_error("An attempt to set the same render camera");

        mCamera = std::move(camera);
        mContext->setCamera(mCamera.get());
    }

    void RenderEngine::setRenderDevice(RefCounted<IRenderDevice> device) {
        if (device == nullptr)
            throw std::runtime_error("An attempt to set null render device");

        mRenderDevice = std::move(device);
        mContext->setRenderDevice(mRenderDevice.get());
    }

    void RenderEngine::setTargetSurface(ID<ignimbrite::IRenderDevice::Surface> surface) {
        CHECK_DEVICE_PRESENT();

        if (surface == mTargetSurface)
            throw std::runtime_error("An attempt to set the same target surface");

        if (surface.isNull())
            throw std::runtime_error("An attempt to set null target surface");

        uint32 width, height;
        mRenderDevice->getSurfaceSize(surface, width, height);
        mOffscreenTarget = std::make_shared<RenderTarget>(mRenderDevice);
        mOffscreenTarget->createTargetFromFormat(width, height, RenderTarget::DefaultFormat::Color0AndDepthStencil);
        mTargetSurface = surface;
    }

    void RenderEngine::setRenderArea(uint32 x, uint32 y, uint32 w, uint32 h) {
        mRenderArea.x = x;
        mRenderArea.y = y;
        mRenderArea.w = w;
        mRenderArea.h = h;

    }

    void RenderEngine::addRenderable(RefCounted<IRenderable> object) {
        auto found = std::find(mRenderObjects.begin(), mRenderObjects.end(), object);

        if (found != mRenderObjects.end())
            throw std::runtime_error("Engine already contains this renderable object");

        uint32 layer = object->getLayerID();
        IRenderable* objectPtr = object.get();
        mRenderLayers[layer].emplace_back(objectPtr);

        mRenderObjects.emplace_back(std::move(object));
    }

    void RenderEngine::removeRenderable(const RefCounted <ignimbrite::IRenderable> &object) {
        auto found = std::find(mRenderObjects.begin(), mRenderObjects.end(), object);

        if (found == mRenderObjects.end())
            throw std::runtime_error("Engine does not contain such renderable object");

        uint32 layer = object->getLayerID();
        IRenderable* objectPtr = object.get();
        auto& list = mRenderLayers[layer];
        auto toRemove = std::find(list.begin(), list.end(), objectPtr);
        list.erase(toRemove);

        mRenderObjects.erase(found);
    }

    void RenderEngine::addLightSource(RefCounted<Light> light) {
        auto found = std::find(mLightSources.begin(), mLightSources.end(), light);

        if (found != mLightSources.end())
            throw std::runtime_error("Engine already contains this light object");

        mLightSources.emplace_back(std::move(light));
    }

    void RenderEngine::removeLightSource(const RefCounted <ignimbrite::Light> &light) {
        auto found = std::find(mLightSources.begin(), mLightSources.end(), light);

        if (found == mLightSources.end())
            throw std::runtime_error("Engine does not contain such light object");

        mLightSources.erase(found);
    }

    void RenderEngine::draw() {
        CHECK_CAMERA_PRESENT();
        CHECK_DEVICE_PRESENT();
        CHECK_SURFACE_PRESENT();

        // Draw consists of 4 main stages
        // 1. Generate shadow maps and do all the pre-render steps
        // 2. Render objects one by one for each layer
        // 3. Run post processing on generated image
        // 4. Present image

        // todo: shadow mapping

        // todo: main pass

        Vec3f cameraPos = mCamera->getPosition();
        const auto& frustum = mCamera->getFrustum();

        for (const auto& layer: mRenderLayers) {
            const auto& list = layer.second;

            if (list.empty())
                continue;

            mCollectQueue.clear();
            mVisibleSortedQueue.clear();

            for (auto object: list) {
                // object not visible at all
                if (!object->isVisible())
                    continue;

                Vec3f pos = object->getWorldPosition();
                float32 maxViewDistanceSq = object->getMaxViewDistanceSquared();
                float32 distanceSq = glm::distance2(cameraPos, pos);

                // Object too far and we can cull it
                if (distanceSq > maxViewDistanceSq && object->canApplyCulling())
                    continue;

                RenderQueueElement element = {};
                element.object = object;
                element.viewDistance = std::sqrt(distanceSq);
                element.boundingBox = object->getWorldBoundingBox();

                mCollectQueue.push_back(element);
            }

            // Do frustum culling
            for (const auto& element: mCollectQueue) {
                if (frustum.isInside(element.boundingBox))
                    mVisibleSortedQueue.push_back(element);
            }

            // Notify elements entered the render queue successfully and get it material for rendering
            for (auto& element: mVisibleSortedQueue) {
                element.object->onRenderQueueEntered(element.viewDistance);
                element.material = element.object->getRenderMaterial();
            }

            // Sort with distance and material predicate
            RenderQueueElement::SortPredicate predicate;
            std::sort(mVisibleSortedQueue.begin(), mVisibleSortedQueue.end(), predicate);

            // todo: Bind render target not surface
            {

                mRenderDevice->drawListBegin();
                mRenderDevice->drawListBindSurface(mTargetSurface,
                        IRenderDevice::Color{0,0,0,0},
                        IRenderDevice::Region{mRenderArea.x, mRenderArea.y,
                                              IRenderDevice::Extent{mRenderArea.w, mRenderArea.h}});
                PipelineContext::cacheSurfaceBinding(mTargetSurface);

                // Pass to object render context and call render for each
                for (const auto& element: mVisibleSortedQueue) {
                    element.object->onRender(*mContext);
                }

                mRenderDevice->drawListEnd();
                mRenderDevice->flush();
                mRenderDevice->synchronize();
                mRenderDevice->swapBuffers(mTargetSurface);
            }


        }

        // todo: post processing

        // todo: presenting
    }

    const String &RenderEngine::getName() {
        static String gEngineName = "RenderEngine";
        return gEngineName;
    }

    void RenderEngine::CHECK_CAMERA_PRESENT() const {
        if (mCamera == nullptr)
            throw std::runtime_error("Camera is not specified");
    }

    void RenderEngine::CHECK_DEVICE_PRESENT() const {
        if (mRenderDevice== nullptr)
            throw std::runtime_error("Render Device is not specified");
    }

    void RenderEngine::CHECK_SURFACE_PRESENT() const {
        if (mTargetSurface.isNull())
            throw std::runtime_error("Target Surface is not specified");
    }

}