/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_CANVAS_H
#define IGNIMBRITE_CANVAS_H

#include <IncludeMath.h>
#include <IRenderDevice.h>
#include <Camera.h>
#include <GraphicsPipeline.h>
#include <Material.h>

namespace ignimbrite {

    class Canvas {
    public:
        explicit Canvas(RefCounted<IRenderDevice> device);
        ~Canvas();

        /**
         * Set surface to render in.
         * Note: to render in different surfaces/targets each of them
         * should be associated with its own instance of Canvas class.
         * It's possible to use one instance, render() or clear() functions
         * must be called before setSurface(..) and setTargetFormat(..),
         * however this will cause pipeline recreation each time when new surface/target will be set.
         */
        void setSurface(ID<IRenderDevice::Surface> surface);
        void setTargetFormat(RefCounted<RenderTarget::Format> format);

        /**
         * Set camera for three dimensional points, lines etc.
         * If only two dimensional primitives will be used,
         * camera setting can be skipped.
         */
        void setCamera(RefCounted<Camera> camera);

        /**
         * Add line for drawing queue of to the current frame.
         * Note: call render() after adding all primitives.
         * Note: requires setCamera(..) to be called.
         */
        void addLine3d(const Vec3f &p0, const Vec3f &p1, const Vec4f &color, float width = 1);
        /**
         * Add line for drawing queue of to the current frame.
         * Note: call render() after adding all primitives.
         */
        void addLine2d(const Vec2f &p0, const Vec2f &p1, const Vec4f &color, float width = 1);

        /**
         * Add line for drawing queue of to the current frame.
         * Note: call render() after adding all primitives.
         */
        void addPoint2d(const Vec2f &p, const Vec4f &color, float size = 4);
        /**
         * Add line for drawing queue of to the current frame.
         * Note: call render() after adding all primitives.
         * Note: requires setCamera(..) to be called.
         */
        void addPoint3d(const Vec3f &p, const Vec4f &color, float size = 4);

        /**
         * Does the actual rendering through drawList commands.
         * Note: should be called only once in a frame.
         * Note: surface/target that were set by setSurface(..) or setTargetFormat(..)
         * must be bound in render device before calling this function.
         * Note: removes all primitives that were added by "add*" functions.
         */
        void render();

        /**
         * Remove all primitives that were added by "add*" functions.
         * Note: render() calls this function after rendering.
         */
        void clear();

    private:
        /** Special struct for vertices that is used in the shader */
        struct Point {
            Vec4f posScale;
            // each component is in [0..1]
            Vec4f color;
        };

        enum class TargetType {
            None, Surface, Framebuffer
        };

        /**
         * Allocate and set params for pipelines. Pipelines will be created and released
         */
        void preparePipelines();
        void createPipelines();

        void renderPrimitives(const std::vector<Point> &ps2d, const std::vector<Point> &ps3d, ID <IRenderDevice::VertexBuffer> vb,
                              const RefCounted<Material> &mat2d, const RefCounted<Material> &mat3d);

    private:
        // per-frame storage for vertices
        std::vector<Point> mPoints2d;
        std::vector<Point> mPoints3d;

        // every 2 points are a line ends
        std::vector<Point> mLines2d;
        std::vector<Point> mLines3d;

        RefCounted<IRenderDevice> mDevice;
        RefCounted<Camera> mCamera;

        TargetType mCurrentTarget = TargetType::None;
        ID<IRenderDevice::Surface> mSurface;
        RefCounted<RenderTarget::Format> mTargetFormat;

        // amount of vertices in mVertexBufferPoints
        uint32 mPointsVertCount = 0;
        ID<IRenderDevice::VertexBuffer> mVertexBufferPoints;

        // amount of vertices in mVertexBufferLines
        uint32 mLinesVertCount = 0;
        ID<IRenderDevice::VertexBuffer> mVertexBufferLines;

        RefCounted<GraphicsPipeline> mPipelinePoints;
        RefCounted<GraphicsPipeline> mPipelineLines;

        RefCounted<Material> mMaterialPoints2d;
        RefCounted<Material> mMaterialLines2d;
        RefCounted<Material> mMaterialPoints3d;
        RefCounted<Material> mMaterialLines3d;
    };
}


#endif //IGNIMBRITE_CANVAS_H
