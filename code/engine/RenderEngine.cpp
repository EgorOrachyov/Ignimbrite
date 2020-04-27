/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <Geometry.h>
#include <RenderEngine.h>
#include <PipelineContext.h>

namespace ignimbrite {

    RenderEngine::RenderEngine() {
        mContext = std::make_shared<IRenderContext>();
    }

    RenderEngine::~RenderEngine() {
        if (mFullscreenQuad.isNotNull()) {
            mRenderDevice->destroyVertexBuffer(mFullscreenQuad);
            mFullscreenQuad = ID<IRenderDevice::VertexBuffer>();
        }
    }

    void RenderEngine::setCamera(RefCounted<Camera> camera) {
        if (mCamera == camera)
            throw std::runtime_error("An attempt to set the same render camera");

        mCamera = std::move(camera);
        mContext->setCamera(mCamera.get());

        if (mCanvas) {
            mCanvas->setCamera(mCamera);
        }
    }

    void RenderEngine::setRenderDevice(RefCounted<IRenderDevice> device) {
        if (device == nullptr)
            throw std::runtime_error("An attempt to set null render device");

        mRenderDevice = std::move(device);
        mContext->setRenderDevice(mRenderDevice.get());

        mCanvas = std::make_shared<Canvas>(mRenderDevice);
        if (mTargetSurface.isNotNull()) {
            mCanvas->setSurface(mTargetSurface);
        }
        if (mCamera) {
            mCanvas->setCamera(mCamera);
        }
    }

    void RenderEngine::setTargetSurface(ID<IRenderDevice::Surface> surface) {
        CHECK_DEVICE_PRESENT();

        if (surface == mTargetSurface)
            throw std::runtime_error("An attempt to set the same target surface");

        if (surface.isNull())
            throw std::runtime_error("An attempt to set null target surface");

        uint32 width, height;
        mRenderDevice->getSurfaceSize(surface, width, height);

        mOffscreenTarget1 = std::make_shared<RenderTarget>(mRenderDevice);
        mOffscreenTarget1->createTargetFromFormat(width, height, RenderTarget::DefaultFormat::Color0AndDepthStencil);
        mOffscreenTarget2 = std::make_shared<RenderTarget>(mRenderDevice);
        mOffscreenTarget2->createTargetFromFormat(width, height, RenderTarget::DefaultFormat::Color0AndDepthStencil);

        auto sampler = std::make_shared<Sampler>(mRenderDevice);
        sampler->setHighQualityFiltering();

        mOffscreenTarget1->getAttachment(0)->setSampler(sampler);
        mOffscreenTarget2->getAttachment(0)->setSampler(sampler);

        mTargetSurface = surface;

        if (mCanvas) {
            mCanvas->setSurface(mTargetSurface);
        }
    }

    void RenderEngine::setShadowTarget(RefCounted<Light> light, RefCounted<RenderTarget> target) {
        // Now only one light cast shadows, but this could be fixed via map (light -> target)
        mShadowsRenderTarget = std::move(target);
        mShadowTargetFormat = mShadowsRenderTarget->getFramebufferFormat();
        mContext->setShadowsRenderTarget(mShadowsRenderTarget.get());
    }

    void RenderEngine::setRenderArea(uint32 x, uint32 y, uint32 w, uint32 h) {
        mRenderArea.x = x;
        mRenderArea.y = y;
        mRenderArea.w = w;
        mRenderArea.h = h;
    }

    void RenderEngine::setPresentationPass(RefCounted<Material> present) {
        CHECK_DEVICE_PRESENT();

        mPresentationMaterial = std::move(present);
        Geometry::createFullscreenQuad(mFullscreenQuad, mRenderDevice);
    }

    void RenderEngine::addRenderable(RefCounted<IRenderable> object) {
        auto found = std::find(mRenderObjects.begin(), mRenderObjects.end(), object);

        if (found != mRenderObjects.end())
            throw std::runtime_error("Engine already contains this renderable object");

        uint32 layer = object->getLayerID();
        IRenderable* objectPtr = object.get();
        mRenderLayers[layer].emplace_back(objectPtr);

        object->onAddToScene(*mContext);
        mRenderObjects.emplace_back(std::move(object));
    }

    void RenderEngine::removeRenderable(const RefCounted <IRenderable> &object) {
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

    void RenderEngine::removeLightSource(const RefCounted <Light> &light) {
        auto found = std::find(mLightSources.begin(), mLightSources.end(), light);

        if (found == mLightSources.end())
            throw std::runtime_error("Engine does not contain such light object");

        mLightSources.erase(found);
    }

    void RenderEngine::addPostEffect(RefCounted<IPostEffect> effect) {
        auto found = std::find(mPostEffects.begin(), mPostEffects.end(), effect);

        if (found != mPostEffects.end())
            throw std::runtime_error("Engine already contains this effect object");

        effect->onAddedToPipeline(mOffscreenTarget1->getFramebufferFormat());
        mPostEffects.emplace_back(std::move(effect));
    }

    void RenderEngine::removePostEffect(const RefCounted<IPostEffect> &effect) {
        auto found = std::find(mPostEffects.begin(), mPostEffects.end(), effect);

        if (found == mPostEffects.end())
            throw std::runtime_error("Engine does not contain such effect object");

        mPostEffects.erase(found);
    }

    void RenderEngine::draw() {
        CHECK_CAMERA_PRESENT();
        CHECK_DEVICE_PRESENT();
        CHECK_SURFACE_PRESENT();
        CHECK_FINAL_PASS_PRESENT();

        // This target will be finally presented to the screen
        RefCounted<RenderTarget> resultPostEffectsPass;

        // Draw consists of 4 main stages
        // 1. Generate shadow maps and do all the pre-render steps
        // 2. Render objects one by one for each layer
        // 3. Run post processing on generated image
        // 4. Present image

        mRenderDevice->drawListBegin();

        Vec3f cameraPos = mCamera->getPosition();
        const auto& frustum = mCamera->getFrustum();

        // todo: make shadow distance variable ?
        float shadowDistance = 20.0f;
        Frustum frustumCut = frustum;
        frustumCut.cutFrustum(shadowDistance / mCamera->getFarClip());

        for (auto &light : mLightSources) {
            if (!light->castShadow()) {
                break;
            }

            mContext->setGlobalLight(light.get());

            Vec3f lightpos = light->getPosition();

            light->buildViewFrustum(frustumCut);
            const auto &lightFrustum = light->getFrustum();

            for (const auto& layer: mRenderLayers) {
                const auto& list = layer.second;

                if (list.empty())
                    continue;

                mCollectQueue.clear();
                mVisibleSortedQueue.clear();

                for (auto object: list) {
                    // object not visible at all or doesn't cast shadows
                    if (!object->isVisible() || !object->castShadows())
                        continue;

                    Vec3f pos = object->getWorldPosition();
                    float32 maxViewDistanceSq = object->getMaxViewDistanceSquared();
                    float32 distanceSq = glm::distance2(lightpos, pos);

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
                    if (lightFrustum.isInside(element.boundingBox))
                        mVisibleSortedQueue.push_back(element);
                }

                // Notify elements entered the render queue successfully and get it material for rendering
                for (auto& element: mVisibleSortedQueue) {
                    element.object->onShadowRenderQueueEntered(element.viewDistance);
                    element.material = element.object->getShadowRenderMaterial();
                }

                // Sort with distance and material predicate
                RenderQueueElement::SortPredicate predicate;
                std::sort(mVisibleSortedQueue.begin(), mVisibleSortedQueue.end(), predicate);

                {
                    IRenderDevice::Region region = {0, 0, {mShadowsRenderTarget->getWidth(), mShadowsRenderTarget->getHeight()}};
                    std::vector<IRenderDevice::Color> clearColors;

                    mRenderDevice->drawListBindFramebuffer(mShadowsRenderTarget->getHandle(), clearColors, region);
                    PipelineContext::cacheFramebufferBinding(mShadowsRenderTarget->getHandle());
                    PipelineContext::cachePipelineBinding(ID<IRenderDevice::GraphicsPipeline>());

                    for (const auto& element: mVisibleSortedQueue) {
                        element.object->onShadowRender(*mContext);
                    }
                }
            }

            // only 1 light casts shadows
            break;
        }

        // todo: main pass

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

            {
                static std::vector<IRenderDevice::Color> clearColors = { IRenderDevice::Color{0,0,0,0} };
                IRenderDevice::Region region = { mRenderArea.x, mRenderArea.y, { mRenderArea.w, mRenderArea.h } };

                //mRenderDevice->drawListBegin();
                mRenderDevice->drawListBindFramebuffer(mOffscreenTarget1->getHandle(), clearColors, region);
                PipelineContext::cacheFramebufferBinding(mOffscreenTarget1->getHandle());

                // Pass to object render context and call render for each
                for (const auto& element: mVisibleSortedQueue) {
                    element.object->onRender(*mContext);
                }
            }
        }

        {
            auto source = mOffscreenTarget1;
            auto dest = mOffscreenTarget2;

            for (auto& effect: mPostEffects) {
                effect->execute(source, dest);
                std::swap(source, dest);
            }

            resultPostEffectsPass = source;
        }

        {
            IRenderDevice::Color color = {0.0f, 0.0f, 0.0f, 0.0f};
            IRenderDevice::Region region = {mRenderArea.x, mRenderArea.y, {mRenderArea.w, mRenderArea.h}};

            const auto &resultFrame = resultPostEffectsPass->getAttachment(0);
            static String texture0 = "Texture0";
            mPresentationMaterial->setTexture2D(texture0, resultFrame);
            mPresentationMaterial->updateUniformData();

            mRenderDevice->drawListBindSurface(mTargetSurface, color, region);
            PipelineContext::cacheSurfaceBinding(mTargetSurface);
            mPresentationMaterial->bindGraphicsPipeline();
            mPresentationMaterial->bindUniformData();
            mRenderDevice->drawListBindVertexBuffer(mFullscreenQuad, 0, 0);
            mRenderDevice->drawListDraw(6, 1);
        }

        mCanvas->render();

        mRenderDevice->drawListEnd();

        mRenderDevice->flush();
        mRenderDevice->synchronize();
        mRenderDevice->swapBuffers(mTargetSurface);
    }

    const RefCounted<ignimbrite::RenderTarget::Format> &RenderEngine::getShadowTargetFormat() const {
        return mShadowTargetFormat;
    }

    const RefCounted <RenderTarget::Format> &RenderEngine::getOffscreenTargetFormat() const {
        return mOffscreenTarget1->getFramebufferFormat();
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

    void RenderEngine::CHECK_FINAL_PASS_PRESENT() const {
        if (mPresentationMaterial == nullptr)
            throw std::runtime_error("Presentation material is not specified");
    }

    void RenderEngine::addScreenPoint2d(const Vec2f &p, const Vec4f &color, float size) {
        mCanvas->addPoint2d(p, color, size);
    }

    void RenderEngine::addScreenLine2d(const Vec2f &a, const Vec2f &b, const Vec4f &color, float width) {
        mCanvas->addLine2d(a, b, color, width);
    }

    void RenderEngine::addPoint3d(const Vec3f &p, const Vec4f &color, float size) {
        mCanvas->addPoint3d(p, color, size);
    }

    void RenderEngine::addLine3d(const Vec3f &a, const Vec3f &b, const Vec4f &color, float width) {
        mCanvas->addLine3d(a, b, color, width);
    }

}