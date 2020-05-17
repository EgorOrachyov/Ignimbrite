/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019, 2020  Egor Orachyov                                        */
/* Copyright (c) 2019, 2020  Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_IPOSTEFFECT_H
#define IGNIMBRITE_IPOSTEFFECT_H

#include <Types.h>
#include <RenderTarget.h>

namespace ignimbrite {

        class IPostEffect {
        public:

            virtual ~IPostEffect() = default;

            /** @return True, if effect must be applied for current pass */
            virtual bool isActive() const = 0;

            /** Called when effect added to the engine post effects pipeline */
            virtual void onAddedToPipeline(const RefCounted<RenderTarget::Format> &targetFormat) = 0;

            /**
             * Called each frame to execute effect if active
             * @param input Input target for pixel data read
             * @param output Output target for pixel data write
             */
            virtual void execute(RefCounted<RenderTarget> &input, RefCounted<RenderTarget> &output) = 0;

        };

}

#endif //IGNIMBRITE_IPOSTEFFECT_H