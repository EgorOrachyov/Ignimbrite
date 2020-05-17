/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERABLEMESH_H
#define IGNIMBRITE_RENDERABLEMESH_H

#include <IRenderable.h>
#include <IncludeStd.h>
#include <Mesh.h>

namespace ignimbrite {

    class RenderableMesh : public IRenderable {
    public:

        ~RenderableMesh() override;

        void setRenderDevice(RefCounted<IRenderDevice> device);
        void setRenderMesh(RefCounted<Mesh> mesh, bool useAsShadowMesh = false);
        void setRenderMaterial(RefCounted<Material> material, bool useAsShadowMaterial = false);
        void setShadowRenderMesh(RefCounted<Mesh> mesh);
        void setShadowRenderMaterial(RefCounted<Material> material);
        void create();

        void rotate(const Vec3f& axis, float32 angle);
        void translate(const Vec3f& translation);
        void setScale(const Vec3f& scale);
        void updateAABB();
        void generateGpuBuffers();
        void updateGpuBuffersData();
        void releaseGpuBuffers();

        // IRenderable

        void onAddToScene(const IRenderContext &context) override;
        void onRenderQueueEntered(float32 distFromViewPoint) override;
        void onRender(const IRenderContext &context) override;
        void onShadowRenderQueueEntered(float32 distFromViewPoint) override;
        void onShadowRender(const IRenderContext &context) override;

        Vec3f getWorldPosition() const override;
        AABB getWorldBoundingBox() const override;
        Material *getRenderMaterial() override;
        Material *getShadowRenderMaterial() override;

    protected:

        bool isDirty() { return mDirty; }
        void markDirty() { mDirty = true; }
        void markClear() { mDirty = false; }

        bool  mDirty = true;
        AABB  mAABB;
        Mat4f mRotation     = Mat4f(1.0f);
        Vec3f mWorldPosition = Vec3f(0,0,0);
        Vec3f mScale = Vec3f(1,1,1);

        RefCounted<Mesh>     mRenderMesh;
        RefCounted<Mesh>     mShadowMesh;
        RefCounted<Material> mRenderMaterial;
        RefCounted<Material> mShadowMaterial;

        RefCounted<IRenderDevice>       mDevice;
        ID<IRenderDevice::IndexBuffer>  mIndexBuffer;
        ID<IRenderDevice::VertexBuffer> mVertexBuffer;
        ID<IRenderDevice::IndexBuffer>  mShadowIndexBuffer;
        ID<IRenderDevice::VertexBuffer> mShadowVertexBuffer;
    };

}

#endif //IGNIMBRITE_RENDERABLEMESH_H