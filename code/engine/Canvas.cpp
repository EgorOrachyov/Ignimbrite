/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/

#include "Canvas.h"
#include <fstream>

namespace ignimbrite {

    Canvas::Canvas(RefCounted<IRenderDevice> device) {
        mDevice = std::move(device);
        preparePipelines();
    }

    void Canvas::preparePipelines() {
        mPipelinePoints = std::make_shared<GraphicsPipeline>(mDevice);
        mPipelineLines = std::make_shared<GraphicsPipeline>(mDevice);

        // create vertex layout
        IRenderDevice::VertexBufferLayoutDesc vertLayout = {};
        vertLayout.usage = VertexUsage::PerVertex;
        vertLayout.stride = sizeof(Point);
        vertLayout.attributes.push_back({0, 0, DataFormat::R32G32B32A32_SFLOAT});
        vertLayout.attributes.push_back({1, sizeof(float) * 4, DataFormat::R32G32B32A32_SFLOAT});

        // init shader
        std::ifstream vertFile("shaders/spirv/CanvasPrimitiveVert.spv", std::ios::binary);
        std::ifstream fragFile("shaders/spirv/CanvasPrimitiveFrag.spv", std::ios::binary);
        std::vector<uint8> vertSpv(std::istreambuf_iterator<char>(vertFile), {});
        std::vector<uint8> fragSpv(std::istreambuf_iterator<char>(fragFile), {});
        RefCounted<Shader> shader = std::make_shared<Shader>(mDevice);
        shader->fromSources(ShaderLanguage::SPIRV, vertSpv, fragSpv);
        shader->reflectData();
        shader->generateUniformLayout();

        GraphicsPipeline *pipelines[] = {mPipelinePoints.get(), mPipelineLines.get()};

        for (GraphicsPipeline *pipeline : pipelines) {
            pipeline->setShader(shader);
            pipeline->setVertexBuffersCount(1);
            pipeline->setVertexBufferDesc(0, vertLayout);
            pipeline->setLineWidth(1.0f);
            pipeline->setBlendEnable(false);
            // depth test and write is disabled by default
            pipeline->setDepthTestEnable(false);
            pipeline->setDepthWriteEnable(false);
            // do not create pipeline now, it will happen when surface or target wil be set
        }

        mPipelinePoints->setPolygonMode(PolygonMode::Point);
        mPipelinePoints->setPrimitiveTopology(PrimitiveTopology::PointList);
        mPipelineLines->setPolygonMode(PolygonMode::Line);
        mPipelineLines->setPrimitiveTopology(PrimitiveTopology::LineList);
    }

    Canvas::~Canvas() {
        if (mVertexBufferPoints.isNotNull()) {
            mDevice->destroyVertexBuffer(mVertexBufferPoints);
        }

        if (mVertexBufferLines.isNotNull()) {
            mDevice->destroyVertexBuffer(mVertexBufferLines);
        }
    }

    void Canvas::setSurface(ID<IRenderDevice::Surface> surface) {
        if (mSurface == surface && mCurrentTarget == TargetType::Surface) {
            return;
        }

        if (!mLines2d.empty() || !mLines3d.empty() || !mPoints2d.empty() || !mPoints3d.empty()) {
            throw std::runtime_error("Target can't be changed if previous surface/target were set and "
                                     "there are primitives to render in the current frame.");
        }

        mSurface = surface;

        // release previous pipeline (if there wasn't, releasePipeline does nothing)
        mPipelinePoints->releasePipeline();
        mPipelineLines->releasePipeline();

        mPipelinePoints->setSurface(mSurface);
        mPipelineLines->setSurface(mSurface);

        createPipelines();

        mCurrentTarget = TargetType::Surface;
    }

    void Canvas::setTargetFormat(RefCounted<RenderTarget::Format> format) {
        if (mTargetFormat == format && mCurrentTarget == TargetType::Framebuffer) {
            return;
        }

        if (!mLines2d.empty() || !mLines3d.empty() || !mPoints2d.empty() || !mPoints3d.empty()) {
            throw std::runtime_error("Target can't be changed if previous surface/target were set and "
                                     "there are primitives to render in the current frame.");
        }

        mTargetFormat = std::move(format);

        // release previous pipeline (if there wasn't, releasePipeline does nothing)
        mPipelinePoints->releasePipeline();
        mPipelineLines->releasePipeline();

        mPipelinePoints->setTargetFormat(mTargetFormat);
        mPipelineLines->setTargetFormat(mTargetFormat);

        createPipelines();

        mCurrentTarget = TargetType::Framebuffer;
    }

    void Canvas::createPipelines() {
        mPipelinePoints->createPipeline();
        mPipelineLines->createPipeline();

        mMaterialPoints3d = std::make_shared<Material>(mDevice);
        mMaterialPoints3d->setGraphicsPipeline(mPipelinePoints);
        mMaterialPoints3d->createMaterial();
        mMaterialLines3d = std::make_shared<Material>(mDevice);
        mMaterialLines3d->setGraphicsPipeline(mPipelineLines);
        mMaterialLines3d->createMaterial();

        mMaterialPoints2d = std::make_shared<Material>(mDevice);
        mMaterialPoints2d->setGraphicsPipeline(mPipelinePoints);
        mMaterialPoints2d->createMaterial();
        mMaterialLines2d = std::make_shared<Material>(mDevice);
        mMaterialLines2d->setGraphicsPipeline(mPipelineLines);
        mMaterialLines2d->createMaterial();

        mMaterialPoints2d->setMat4("UBO.vp", Mat4f(1.0f));
        mMaterialPoints2d->updateUniformData();
        mMaterialLines2d->setMat4("UBO.vp", Mat4f(1.0f));
        mMaterialLines2d->updateUniformData();
    }

    void Canvas::setCamera(RefCounted<Camera> camera) {
        mCamera = std::move(camera);
    }

    void Canvas::addLine2d(const Vec2f &p0, const Vec2f &p1, const Vec4f &color, float width) {
        mLines2d.push_back({Vec4f(p0, 0, width), color});
        mLines2d.push_back({Vec4f(p1, 0, width), color});
    }

    void Canvas::addLine3d(const Vec3f &p0, const Vec3f &p1, const Vec4f &color, float width) {
        if ((!mLines3d.empty() || !mPoints3d.empty()) && !mCamera) {
            throw std::runtime_error("Camera must be set before rendering three dimensional canvas primitives");
        }

        mLines3d.push_back({Vec4f(p0, width), color});
        mLines3d.push_back({Vec4f(p1, width), color});
    }

    void Canvas::addPoint2d(const Vec2f &p, const Vec4f &color, float size) {
        mPoints2d.push_back({Vec4f(p, 0, size), color});
    }

    void Canvas::addPoint3d(const Vec3f &p, const Vec4f &color, float size) {
        if ((!mLines3d.empty() || !mPoints3d.empty()) && !mCamera) {
            throw std::runtime_error("Camera must be set before rendering three dimensional canvas primitives");
        }

        mPoints3d.push_back({Vec4f(p, size), color});
    }

    void Canvas::renderPrimitives(const std::vector<Point> &ps2d, const std::vector<Point> &ps3d, ID<IRenderDevice::VertexBuffer> &vb,
                                  const RefCounted<Material> &mat2d, const RefCounted<Material> &mat3d) {

        if (ps2d.size() + ps3d.size() > mLinesVertCount) {
            if (vb.isNotNull()) {
                mDevice->destroyVertexBuffer(vb);
            }

            uint32 vbSize = sizeof(Point) * (ps2d.size() + ps3d.size());
            vbSize = (uint32) (pow(2, ceil(log(vbSize * 1.3) / log(2))));
            vb = mDevice->createVertexBuffer(BufferUsage::Dynamic, vbSize, nullptr);
        }

        if (!ps2d.empty()) {
            mDevice->updateVertexBuffer(vb, sizeof(Point) * ps2d.size(), 0, ps2d.data());

            mat2d->bindUniformData();

            mDevice->drawListBindVertexBuffer(vb, 0, 0);
            mDevice->drawListDraw(ps2d.size(), 1);
        }

        if (!ps3d.empty()) {
            mDevice->updateVertexBuffer(vb, sizeof(Point) * ps3d.size(),sizeof(Point) * ps2d.size(), ps3d.data());

            mat3d->setMat4("UBO.vp", mCamera->getViewProjClipMatrix());
            mat3d->updateUniformData();
            mat3d->bindUniformData();

            mDevice->drawListBindVertexBuffer(vb, 0, sizeof(Point) * ps3d.size());
            mDevice->drawListDraw(ps3d.size(), 1);
        }
    }

    void Canvas::render(){
        if (mCurrentTarget == TargetType::None) {
            throw std::runtime_error("Surface or target isn't set for Canvas");
        }

        mPipelinePoints->bindPipeline();
        renderPrimitives(mPoints2d, mPoints3d, mVertexBufferPoints, mMaterialPoints2d, mMaterialPoints3d);

        mPipelineLines->bindPipeline();
        renderPrimitives(mLines2d, mLines3d, mVertexBufferLines, mMaterialLines2d, mMaterialLines3d);

        clear();
    }

    void Canvas::clear() {
        mPoints2d.clear();
        mPoints3d.clear();
        mLines2d.clear();
        mLines3d.clear();
    }
}

