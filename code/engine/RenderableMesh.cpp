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
    
    void RenderableMesh::setShadowRenderMesh(RefCounted<Mesh> mesh) {
        if (mesh == nullptr)
            throw std::runtime_error("An attempt to set null mesh");

        mShadowMesh = std::move(mesh);
        markDirty();
    }
    
    void RenderableMesh::setShadowRenderMaterial(RefCounted<Material> material) {
        if (material == nullptr)
            throw std::runtime_error("An attempt to set null material");

        mShadowMaterial = std::move(material);
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

    void RenderableMesh::setScale(const Vec3f &scale) {
        mScale = scale;

        markDirty();
    }

    void RenderableMesh::updateAABB() {
        if (!isDirty())
            return;

        std::array<glm::vec3, 8> vertices = {};

        mAABB = mRenderMesh->getBoundingBox();
        mAABB.getVertices(vertices);

        for (auto& v: vertices) {
            v[0] *= mScale[0];
            v[1] *= mScale[1];
            v[2] *= mScale[2];

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

        if (mShadowVertexBuffer.isNotNull())
            throw std::runtime_error("An attempt to recreate buffer");

        if (mShadowIndexBuffer.isNotNull())
            throw std::runtime_error("An attempt to recreate buffer");

        vbSize = mShadowMesh->getStride() * mShadowMesh->getVertexCount();
        mShadowVertexBuffer = mDevice->createVertexBuffer(BufferUsage::Static, vbSize, mShadowMesh->getVertexData());

        ibSize = mShadowMesh->getIndicesCount() * sizeof(uint32);
        mShadowIndexBuffer = mDevice->createIndexBuffer(BufferUsage::Static, ibSize, mShadowMesh->getIndexData());
    }

    void RenderableMesh::updateGpuBuffersData() {
        uint32 vbSize = mRenderMesh->getStride() * mRenderMesh->getVertexCount();
        mDevice->updateVertexBuffer(mVertexBuffer, vbSize, 0, mRenderMesh->getVertexData());

        vbSize = mShadowMesh->getStride() * mShadowMesh->getVertexCount();
        mDevice->updateVertexBuffer(mShadowVertexBuffer, vbSize, 0, mShadowMesh->getVertexData());
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
        if (mShadowVertexBuffer.isNotNull()) {
            mDevice->destroyVertexBuffer(mShadowVertexBuffer);
            mShadowVertexBuffer = ID<IRenderDevice::VertexBuffer>();
        }
        if (mShadowIndexBuffer.isNotNull()) {
            mDevice->destroyIndexBuffer(mShadowIndexBuffer);
            mShadowIndexBuffer = ID<IRenderDevice::IndexBuffer>();
        }
    }

    void RenderableMesh::onAddToScene(const IRenderContext &context) {
        // Do nothing
    }

    void RenderableMesh::onRenderQueueEntered(float32 distFromViewPoint) {
        // Possibly select LOD, but this mesh has only LOD 0
        // Do nothing
    }

    void RenderableMesh::onRender(const IRenderContext &context) {
        auto device = context.getRenderDevice();
        auto camera = context.getCamera();
        auto light = context.getGlobalLight();

        auto model = glm::translate(mWorldPosition) * mRotation * glm::scale(mScale);

        auto camViewProj = camera->getViewProjClipMatrix();

        if (light != nullptr) {
            auto lightViewProj = light->getViewProjClipMatrix();

            mRenderMaterial->setMat4("UBO.lightSpace", lightViewProj);
            mRenderMaterial->setVec3("UBO.lightDir", light->getDirection());
            mRenderMaterial->setTexture2D("shadowMap", context.getShadowMap());
        }

        // todo: another bindings
        mRenderMaterial->setMat4("UBO.viewProj", camViewProj);
        mRenderMaterial->setMat4("UBO.model", model);

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
        auto device = context.getRenderDevice();
        auto light = context.getGlobalLight();

        auto model = glm::translate(mWorldPosition) * mRotation * glm::scale(mScale);
        auto lightMVP = light->getViewProjClipMatrix() * model;

        // todo: another bindings
        mShadowMaterial->setMat4("UBO.depthMVP", lightMVP);

        mShadowMaterial->updateUniformData();
        mShadowMaterial->bindGraphicsPipeline();
        mShadowMaterial->bindUniformData();

        device->drawListBindVertexBuffer(mShadowVertexBuffer, 0, 0);
        device->drawListBindIndexBuffer(mShadowIndexBuffer, IndicesType::Uint32, 0);
        device->drawListDrawIndexed(mShadowMesh->getIndicesCount(), 1);
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