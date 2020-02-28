/**********************************************************************************/
/* This file is part of Ignimbrite project                                        */
/* https://github.com/EgorOrachyov/Ignimbrite                                     */
/**********************************************************************************/
/* Licensed under MIT License                                                     */
/* Copyright (c) 2019 - 2020 Egor Orachyov                                        */
/* Copyright (c) 2019 - 2020 Sultim Tsyrendashiev                                 */
/**********************************************************************************/

#include <UniformBuffer.h>

namespace ignimbrite {

    UniformBuffer::UniformBuffer(RefCounted<ignimbrite::RenderDevice> device)
         : mDevice(std::move(device)) {

    }

    UniformBuffer::~UniformBuffer() {
        releaseHandle();
    }

    void UniformBuffer::createBuffer(ignimbrite::uint32 size) {
        if (mHandle.isNull()) {
            mBuffer.resize(size);
            mHandle = mDevice->createUniformBuffer(BufferUsage::Dynamic, size, nullptr);
            if (mHandle.isNull()) {
                // todo: do something
            }
        }
    }

    void UniformBuffer::updateData(ignimbrite::uint32 size, ignimbrite::uint32 offset, const ignimbrite::uint8 *data) {
        updateDataOnCPU(size, offset, data);
        updateDataOnGPU();
    }

    void UniformBuffer::updateDataOnCPU(uint32 size, uint32 offset, const uint8 *data) {
        auto bufferSize = getBufferSize();

        if (size + offset <= bufferSize) {
            auto memory = mBuffer.data();
            memcpy(memory + offset, data, sizeof(uint8) * size);
        }
    }

    void UniformBuffer::updateDataOnGPU() {
        if (mHandle.isNotNull()) {
            mDevice->updateUniformBuffer(mHandle, getBufferSize(), 0, mBuffer.data());
        }
    }

    void UniformBuffer::releaseHandle() {
        if (mHandle.isNotNull()) {
            mDevice->destroyUniformBuffer(mHandle);
            mHandle = ID<RenderDevice::UniformBuffer>();
        }
    }

}
