/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#ifndef IGNIMBRITE_UNIFORMBUFFER_H
#define IGNIMBRITE_UNIFORMBUFFER_H

#include <CacheItem.h>
#include <IRenderDevice.h>
#include <vector>
#include <memory>

namespace ignimbrite {

    class UniformBuffer : public CacheItem {
    public:
        explicit UniformBuffer(RefCounted<IRenderDevice> device);
        ~UniformBuffer() override;

        void createBuffer(uint32 size);
        void updateData(uint32 size, uint32 offset, const uint8* data);
        void updateDataOnCPU(uint32 size, uint32 offset, const uint8* data);
        void updateDataOnGPU();
        void releaseHandle();

        uint32 getBufferSize() const { return (uint32)mBuffer.size(); }
        const std::vector<uint8> &getData() const { return mBuffer; }
        const ID<IRenderDevice::UniformBuffer> &getHandle() const { return mHandle; }
    private:
        /** Data cached on CPU */
        std::vector<uint8> mBuffer;
        /** GPU resource */
        ID<IRenderDevice::UniformBuffer> mHandle;
        /** Device for GPU communication */
        RefCounted<IRenderDevice> mDevice;
    };

}

#endif //IGNIMBRITE_UNIFORMBUFFER_H