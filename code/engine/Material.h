/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_MATERIAL_H
#define IGNIMBRITE_MATERIAL_H

#include <CacheItem.h>
#include <Shader.h>
#include <Texture.h>
#include <UniformBuffer.h>
#include <IncludeMath.h>

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

        explicit Material(RefCounted<RenderDevice> device);
        ~Material() override;

        void create(RefCounted<Shader> shader /** todo: pipeline, layout, etc. */);
        void release();

        ///////////////////////// Uniform data access /////////////////////////

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
        /** Set 2D texture (a.k.a sampler2D) value directly mapped to the GPU uniform params */
        void setTexture2D(const String& name, RefCounted<Texture> texture);

        ///////////////////////// Graphics logic /////////////////////////

        /** Bind this material graphics pipeline as active rendering target */
        void bindGraphicsPipeline();
        /** Bind this material graphics pipeline as active rendering target */
        void bindUniformData();
        /** Writes all the uniform data to uniform buffers on GPU */
        void updateUniformData();

        ///////////////////////// Instancing /////////////////////////

        /** Creates instance of this material, modifiable copy of the one */
        RefCounted<Material> clone() const;

    private:

        /** Data shared among instances of the single material */
        struct SharedData {
            /** Automatically release all resources on refs count == 0 */
            ~SharedData();
            /** Destroy all the GPU resources */
            void release();

            ID<RenderDevice::GraphicsPipeline> pipeline;
            ID<RenderDevice::UniformLayout>    uniformLayout;
            ID<RenderDevice::VertexLayout>     vertexLayout;
            RefCounted<Shader>                 shader;
            RefCounted<RenderDevice>           device;
        };

        bool mUniformBuffersWereModified = false;
        bool mUniformTexturesWereModified = false;

        /** Material data shared among several instances */
        RefCounted<SharedData> mData;
        /** Data, specific for concrete material */
        ID<RenderDevice::UniformSet> mUniformSet;
        std::unordered_map<uint32, UniformBuffer> mUniformBuffers;
        std::unordered_map<uint32, RefCounted<Texture>> mTextures;
    };

}

#endif //IGNIMBRITE_MATERIAL_H