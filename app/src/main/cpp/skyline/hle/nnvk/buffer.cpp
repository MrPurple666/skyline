#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "nnvk.h"
#include "buffer.h"

namespace nnvk {
    BufferBuilder::BufferBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(BufferBuilder);
    }

    void BufferBuilder::SetDevice(Device *pDevice) {
        device = pDevice;
    }

    void BufferBuilder::SetStorage(MemoryPool *pool, i64 offset, u64 size) {
        memoryPool = pool;
        memoryOffset = offset;
        memorySize = size;
    }

    const Device *BufferBuilder::GetDevice() const {
        return device;
    }

    const MemoryPool *BufferBuilder::GetMemoryPool() const {
        return memoryPool;
    }

    i64 BufferBuilder::GetMemoryOffset() const {
        return memoryOffset;
    }

    u64 BufferBuilder::GetSize() const {
        return memorySize;
    }

    /* Wrappers */
    void Context::BufferBuilderSetDefaults(BufferBuilder *builder) {
        new (builder) BufferBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, BufferBuilder, SetDevice, Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, BufferBuilder, SetStorage, MemoryPool *, i64, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, BufferBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const MemoryPool *, BufferBuilder, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, BufferBuilder, GetMemoryOffset)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, BufferBuilder, GetSize)
    /* End wrappers */

    Buffer::Buffer(ApiVersion version, const BufferBuilder &builder)
        :  memoryPool{builder.memoryPool},
           memoryOffset{builder.memoryOffset},
           memorySize{builder.memorySize} {
        NNVK_FILL_VERSIONED_STRUCT(Buffer);
    }

    Buffer::~Buffer() {}

    void Buffer::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void *Buffer::Map() const {
        return static_cast<u8 *>(memoryPool->Map()) + memoryOffset;
    }

    BufferAddress Buffer::GetAddress() const {
        return memoryPool->GetBufferAddress() + static_cast<u64>(memoryOffset);
    }

    void Buffer::FlushMappedRange(i64 offset, u64 size) const {
        memoryPool->FlushMappedRange(offset + memoryOffset, size);
    }

    void Buffer::InvalidateMappedRange(i64 offset, u64 size) const {
        memoryPool->InvalidateMappedRange(offset + memoryOffset, size);
    }

    MemoryPool *Buffer::GetMemoryPool() const {
        return memoryPool;
    }

    i64 Buffer::GetMemoryOffset() const {
        return memoryOffset;
    }

    u64 Buffer::GetSize() const {
        return memorySize;
    }

    u64 Buffer::GetDebugID() const {
        throw std::runtime_error("Buffer::GetDebugID not implemented");
    }

    /* Wrappers */
    bool Context::BufferInitialize(Buffer *pool, const BufferBuilder *builder) {
        new (pool) Buffer(apiVersion, *builder);
        return true;
    }

    void Context::BufferFinalize(Buffer *pool) {
        pool->~Buffer();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Buffer, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, Buffer, Map)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(BufferAddress, Buffer, GetAddress)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Buffer, FlushMappedRange, i64, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Buffer, InvalidateMappedRange, i64, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(MemoryPool *, Buffer, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, Buffer, GetMemoryOffset)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Buffer, GetSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Buffer, GetDebugID)
    /* End wrappers */
}