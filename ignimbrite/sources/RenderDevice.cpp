//
// Created by Egor Orachyov on 2019-10-27.
//

#include <ignimbrite/RenderDevice.h>

namespace ignimbrite {

    const ObjectID RenderDevice::INVALID = ObjectID(0, 0);

    const std::string &RenderDevice::getDeviceName() const {
        static const std::string device = "Default Device";
        return device;
    }

    const std::string &RenderDevice::getVendorName() const {
        static const std::string device = "Default Vendor";
        return device;
    }

} // namespace ignimbrite