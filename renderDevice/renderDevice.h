//
// Created by Egor Orachyov on 2019-10-14.
//

#ifndef VULKANRENDERER_RENDERDEVICE_H
#define VULKANRENDERER_RENDERDEVICE_H

#include <string>

/**
 * Rendering device interface.
 * Wrapper for third-party drawing API, such as vulkan, opengl etc.
 */
class RenderDevice {
public:

    /** @return Readable hardware API name */
    virtual const std::string& getDeviceName() const = 0;
    /** @return Video card vendor name */
    virtual const std::string& getVendor() const = 0;

};

#endif //VULKANRENDERER_RENDERDEVICE_H
