/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_PIPELINECONTEXT_H
#define IGNIMBRITE_PIPELINECONTEXT_H

#include <IncludeStd.h>
#include <IRenderDevice.h>

namespace ignimbrite {

    class PipelineContext {
    public:
        static void cacheFramebufferBinding(ID<IRenderDevice::Framebuffer> framebuffer);
        static void cacheSurfaceBinding(ID<IRenderDevice::Surface> surface);
        static void cachePipelineBinding(ID<IRenderDevice::GraphicsPipeline> pipeline);
        static bool isPipelineCached(ID<IRenderDevice::GraphicsPipeline> pipeline);
    private:
        static ID<IRenderDevice::Surface> mSurfaceBound;
        static ID<IRenderDevice::Framebuffer> mFramebufferBound;
        static ID<IRenderDevice::GraphicsPipeline> mPipelineBound;
    };

}

#endif //IGNIMBRITE_PIPELINECONTEXT_H