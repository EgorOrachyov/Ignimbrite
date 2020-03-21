/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_RENDERQUEUEELEMENT_H
#define IGNIMBRITE_RENDERQUEUEELEMENT_H

#include <IRenderable.h>

namespace ignimbrite {

    struct RenderQueueElement {
        IRenderable* object  = nullptr;
        Material* material   = nullptr;
        float32 viewDistance = 0.0f;
        AABB boundingBox;

        /**
         * Predicate to sort elements of the visible render queue,
         * order by material its materials and then by distance in the same material group.
         */
        struct SortPredicate {
            bool operator()(const RenderQueueElement& a, const RenderQueueElement& b) const {
                auto& m1 = *a.material;
                auto& m2 = *b.material;

                if (m1.getGraphicsPipeline().get() < m2.getGraphicsPipeline().get())
                    return a.viewDistance < b.viewDistance;
                else
                    return false;
            }
        };
    };

}

#endif //IGNIMBRITE_RENDERQUEUEELEMENT_H
