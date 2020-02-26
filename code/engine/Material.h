/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_MATERIAL_H
#define IGNIMBRITE_MATERIAL_H

#include <CacheItem.h>
#include <Shader.h>
#include <Texture.h>
#include <UniformBuffer.h>

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

        ~Material() override = default;


    private:
        std::shared_ptr<Shader> mShader;
        std::vector<UniformBuffer> mUniformBuffers;
        std::vector<std::shared_ptr<Texture>> mTextures;
    };

}

#endif //IGNIMBRITE_MATERIAL_H