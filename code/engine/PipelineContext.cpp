/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <PipelineContext.h>

namespace ignimbrite {

    ID<IRenderDevice::Surface> PipelineContext::mSurfaceBound;
    ID<IRenderDevice::Framebuffer> PipelineContext::mFramebufferBound;
    ID<IRenderDevice::GraphicsPipeline> PipelineContext::mPipelineBound;

    void PipelineContext::cacheFramebufferBinding(ignimbrite::ID<ignimbrite::IRenderDevice::Framebuffer> framebuffer) {
        mFramebufferBound = framebuffer;
        mPipelineBound = ID<IRenderDevice::GraphicsPipeline>();
    }

    void PipelineContext::cacheSurfaceBinding(ignimbrite::ID<ignimbrite::IRenderDevice::Surface> surface) {
        mSurfaceBound = surface;
        mPipelineBound = ID<IRenderDevice::GraphicsPipeline>();
    }

    void PipelineContext::cachePipelineBinding(ignimbrite::ID<ignimbrite::IRenderDevice::GraphicsPipeline> pipeline) {
        mPipelineBound = pipeline;
    }

    bool PipelineContext::isPipelineCached(ignimbrite::ID<ignimbrite::IRenderDevice::GraphicsPipeline> pipeline) {
        return mPipelineBound == pipeline;
    }

}