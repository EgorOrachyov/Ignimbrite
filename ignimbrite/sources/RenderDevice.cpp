/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Berserk                                        */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov, Sultim Tsyrendashiev                  */
/**********************************************************************************/

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