/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <IRenderDevice.h>

namespace ignimbrite {

    IRenderDevice::Type IRenderDevice::getDeviceType() const {
        return Type::Custom;
    }

    const String &IRenderDevice::getDeviceName() const {
        static const String device = "Default Device";
        return device;
    }

    const String &IRenderDevice::getVendorName() const {
        static const String device = "Default Vendor";
        return device;
    }

} // namespace ignimbrite