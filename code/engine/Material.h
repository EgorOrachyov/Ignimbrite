/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_MATERIAL_H
#define IGNIMBRITE_MATERIAL_H

#include <IncludeMath.h>
#include <CacheItem.h>
#include <Shader.h>
#include <Texture.h>
#include <UniformBuffer.h>
#include <GraphicsPipeline.h>

namespace ignimbrite {

    /**
     * @brief Material base class
     *
     * A material defines visual properties of any surface of object.
     * Is conceptual object, which encapsulates inside an gpu program
     * and related uniform data, for actual rendering of the objects.
     *
     * Material itself has no geometry or drawing primitives, it is
     * only defines what data rendered an how it is rendered.
     *
     * Material is composed from:
     * - shader, which defines GPU program with possible reflected
     *   uniform data
     * - pipeline setting, which defines rasterization, blending,
     *   depth and stencil testing operations
     * - uniform set, a particular bindings of concrete data from
     *   CPU to GPU, such as uniform buffers and textures.
     *
     * This material class represents single rendering behaviour.
     * It does not store Techniques or Passes, since they could be
     * created from on top of the existing material class.
     */
    class Material : public CacheItem {
    public:

        explicit Material(RefCounted<IRenderDevice> device);
        ~Material() override;

        void setGraphicsPipeline(RefCounted<GraphicsPipeline> pipeline);

        void createMaterial();
        void releaseMaterial();

        /** Set int value directly mapped to the GPU uniform params */
        void setInt(const String& name, int32 value);
        /** Set float value directly mapped to the GPU uniform params */
        void setFloat(const String& name, float32 value);
        /** Set vec2 value directly mapped to the GPU uniform params */
        void setVec2(const String& name, const Vec2f& vec);
        /** Set vec3 value directly mapped to the GPU uniform params */
        void setVec3(const String& name, const Vec3f& vec);
        /** Set vec4 value directly mapped to the GPU uniform params */
        void setVec4(const String& name, const Vec4f& vec);
        /** Set mat4 value directly mapped to the GPU uniform params */
        void setMat4(const String& name, const Mat4f& mat);
        /** Set texture directly mapped to the GPU uniform params */
        void setTexture(const String& name, RefCounted<Texture> texture);

        /**
         * Set all 2D textures in this material to specified default one.
         * This function can be used to prevent unbound 2D textures.
         * NOTE: cubemaps must be set manually.
         */
        void setAll2DTextures(RefCounted<Texture> defaultTexture);

        /** Bind this material graphics pipeline as active rendering target */
        void bindGraphicsPipeline();
        /** Bind this material graphics pipeline as active rendering target */
        void bindUniformData();
        /** Writes all the uniform data to uniform buffers on GPU */
        void updateUniformData();

        /** Creates instance of this material, modifiable copy of the one */
        RefCounted<Material> clone() const;
        const RefCounted<GraphicsPipeline> &getGraphicsPipeline() const;

    private:

        bool mUniformBuffersWereModified = true;
        bool mUniformTexturesWereModified = true;

        RefCounted<IRenderDevice> mDevice;
        RefCounted<GraphicsPipeline> mPipeline;

        /** Data, specific for concrete material */
        ID<IRenderDevice::UniformSet> mUniformSet;
        std::unordered_map<uint32, UniformBuffer> mUniformBuffers;
        std::unordered_map<uint32, RefCounted<Texture>> mTextures;
    };

}

#endif //IGNIMBRITE_MATERIAL_H