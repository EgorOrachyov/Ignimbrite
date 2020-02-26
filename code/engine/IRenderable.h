/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

#ifndef IGNIMBRITE_IRENDERABLE_H
#define IGNIMBRITE_IRENDERABLE_H

#include <Types.h>
#include <AABB.h>

namespace ignimbrite {

    /**
     * @brief Any visible object
     *
     * This is a base class for any object, which could be queued to the
     * rendering pipeline and rendered in the main rendering pass.
     *
     * For each rendering iteration visibility checks are done for all
     * renderables and then they are sorted by the materials and actually visualised.
     *
     */
    class IRenderable {
    public:

        enum class DefaultLayers : uint32 {
            Background = 0x10,
            Solid = 0x20,
            Transparent = 0x30,
            Overlay = 0x40,
            UI = 0x50
        };

        virtual ~IRenderable() = default;

        /** Called once, when this node enter draw queue after culling and material sorting */
        virtual void onRenderQueueEntered(/* todo: pass useful info */) = 0;
        /** Called once to draw this render node */
        virtual void onRender(/* todo: pass useful info */) = 0;

        void setVisible(bool set = true) { mIsVisible = set; }
        void setCanApplyCulling(bool set = true) { mCanApplyCulling = set; }
        void setMaxViewDistance(float32 distance) { mMaxViewDistance = distance; }
        void setLayerID(uint32 layer) { mLayerID = layer; }

        /** @return True, if object visible and must be submitted for rendering queue */
        bool isVisible() const { return mIsVisible; }
        /** @return True, if can apply culling for that object based on its world position setting */
        bool canApplyCulling() const { return mCanApplyCulling; }
        /** @return Max view distance after this object is automatically culled */
        float32 getMaxViewDistance() const { return mMaxViewDistance; }
        /** @return Layer id of this object. All the objects are grouped by its layer rendered layer by layer */
        uint32 getLayerID() const { return mLayerID; }

    private:
        bool mCanApplyCulling = false;
        bool mIsVisible = false;
        uint32 mLayerID = 0x0;
        float32 mMaxViewDistance = 0.0f;
    };

}

#endif //IGNIMBRITE_IRENDERABLE_H