/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                       */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                */
/**********************************************************************************/


#ifndef IGNIMBRITE_IPRESENTATIONPASS_H
#define IGNIMBRITE_IPRESENTATIONPASS_H

#include <RenderTarget.h>

namespace ignimbrite {

    class IPresentationPass {
    public:
        virtual void present(
                ID<IRenderDevice::Surface> targetSurface, IRenderDevice::Region surfaceRegion,
                RefCounted<RenderTarget> source) = 0;
    };

}

#endif //IGNIMBRITE_IPRESENTATIONPASS_H
