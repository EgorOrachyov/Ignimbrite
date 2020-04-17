/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <RenderableMesh.h>

namespace ignimbrite {

    RenderableMesh::~RenderableMesh() {
        releaseGpuBuffers();
    }

    void RenderableMesh::setRenderDevice(RefCounted<IRenderDevice> device) {
        if (device == nullptr)
            throw std::runtime_error("An attempt to set null device");

        mDevice = std::move(device);
    } 
    
    
    void RenderableMesh::setRenderMesh(RefCounted<Mesh> mesh, bool useAsShadowMesh) {
        if (mesh == nullptr)
            throw std::runtime_error("An attempt to set null mesh");

        mRenderMesh = std::move(mesh);
        if (useAsShadowMesh) mShadowMesh = mRenderMesh;

        markDirty();
    }

    void RenderableMesh::setRenderMaterial(RefCounted<Material> material, bool useAsShadowMaterial) {
        if (material == nullptr)
            throw std::runtime_error("An attempt to set null material");

        mRenderMaterial = std::move(material);
        if (useAsShadowMaterial) mShadowMaterial = mRenderMaterial;

        markDirty();
    }

    void RenderableMesh::create() {
        releaseGpuBuffers();
        generateGpuBuffers();
        updateAABB();
        markClear();
    }

    void RenderableMesh::rotate(const Vec3f &axis, float32 angle) {
        mRotation = glm::rotate(angle, axis) * mRotation;

        markDirty();
    }

    void RenderableMesh::translate(const Vec3f &translation) {
        mWorldPosition += translation;

        markDirty();
    }

    void RenderableMesh::updateAABB() {
        if (!isDirty())
            return;

        std::array<glm::vec3, 8> vertices = {};

        mAABB = mRenderMesh->getBoundingBox();
        mAABB.getVertices(vertices);

        for (auto& v: vertices) {
            auto r = (mRotation * glm::vec4(v, 1.0f));
            v = glm::vec3(r[0], r[1], r[2]) + mWorldPosition;
        }

        mAABB = AABB(vertices);

        markClear();
    }

    void RenderableMesh::generateGpuBuffers() {
        if (mVertexBuffer.isNotNull())
            throw std::runtime_error("An attempt to recreate buffer");


        if (mIndexBuffer.isNotNull())
            throw std::runtime_error("An attempt to recreate buffer");

        uint32 vbSize = mRenderMesh->getStride() * mRenderMesh->getVertexCount();
        mVertexBuffer = mDevice->createVertexBuffer(BufferUsage::Static, vbSize, mRenderMesh->getVertexData());

        uint32 ibSize = mRenderMesh->getIndicesCount() * sizeof(uint32);
        mIndexBuffer = mDevice->createIndexBuffer(BufferUsage::Static, ibSize, mRenderMesh->getIndexData());
    }

    void RenderableMesh::updateGpuBuffersData() {
        uint32 vbSize = mRenderMesh->getStride() * mRenderMesh->getVertexCount();
        mDevice->updateVertexBuffer(mVertexBuffer, vbSize, 0, mRenderMesh->getVertexData());
    }

    void RenderableMesh::releaseGpuBuffers() {
        if (mVertexBuffer.isNotNull()) {
            mDevice->destroyVertexBuffer(mVertexBuffer);
            mVertexBuffer = ID<IRenderDevice::VertexBuffer>();
        }
        if (mIndexBuffer.isNotNull()) {
            mDevice->destroyIndexBuffer(mIndexBuffer);
            mIndexBuffer = ID<IRenderDevice::IndexBuffer>();
        }
    }

    void RenderableMesh::onRenderQueueEntered(float32 distFromViewPoint) {
        // Possibly select LOD, but this mesh has only LOD 0
        // Do nothing
    }

    void RenderableMesh::onRender(const IRenderContext &context) {
        auto device = context.getRenderDevice();
        auto camera = context.getCamera();

        auto Model = glm::translate(mWorldPosition) * mRotation;
        auto MVP = camera->getViewProjClipMatrix() * Model;

        // todo: another bindings
        static String mvpName = "bufferVals.mvp";
        mRenderMaterial->setMat4(mvpName, MVP);
        mRenderMaterial->updateUniformData();
        mRenderMaterial->bindGraphicsPipeline();
        mRenderMaterial->bindUniformData();
        device->drawListBindVertexBuffer(mVertexBuffer, 0, 0);
        device->drawListBindIndexBuffer(mIndexBuffer, IndicesType::Uint32, 0);
        device->drawListDrawIndexed(mRenderMesh->getIndicesCount(), 1);
    }

    void RenderableMesh::onShadowRenderQueueEntered(float32 distFromViewPoint) {
        // Possibly select LOD, but this mesh has only LOD 0
        // Do nothing
    }

    void RenderableMesh::onShadowRender(const IRenderContext &context) {
        auto& device = *context.getRenderDevice();

        // todo: generate vertex / index buffers for shadow mesh
    }

    Vec3f RenderableMesh::getWorldPosition() const {
        return mWorldPosition;
    }

    AABB RenderableMesh::getWorldBoundingBox() const {
        return mAABB;
    }

    Material *RenderableMesh::getRenderMaterial() {
        return mRenderMaterial.get();
    }

    Material *RenderableMesh::getShadowRenderMaterial() {
        return mShadowMaterial.get();
    }
}