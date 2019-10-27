//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_RENDERDEVICE_H
#define VULKANRENDERER_RENDERDEVICE_H

#include <ObjectID.h>
#include <string>

/**
 * Rendering device interface.
 * Wrapper for third-party drawing API, such as Vulkan, OpenGL etc.
 */
class RenderDevice {
public:
    typedef ObjectID ID;

    virtual ~RenderDevice() = default;

    virtual ID createVertexBuffer(uint32 size, void* data) = 0;
    virtual ID createIndexBuffer(uint32 size, void* data) = 0;



    /** @return Readable hardware API name */
    virtual const std::string& getDeviceName() const;
    /** @return Video card vendor name */
    virtual const std::string& getVendor() const;

};

#endif //VULKANRENDERER_RENDERDEVICE_H
