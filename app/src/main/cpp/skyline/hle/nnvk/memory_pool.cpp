#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "logging.h"
#include "memory_manager.h"
#include "nnvk.h"
#include "memory_pool.h"

namespace nnvk {
    MemoryPoolBuilder::MemoryPoolBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(MemoryPoolBuilder);
    }

    void MemoryPoolBuilder::SetDevice(Device *pDevice) {
        device = pDevice;
    }

    void MemoryPoolBuilder::SetStorage(void *pMemory, u64 pSize) {
        storage = {reinterpret_cast<u8 *>(pMemory), pSize};
    }

    void MemoryPoolBuilder::SetFlags(MemoryPoolFlags pFlags) {
        flags = pFlags;
    }

    const Device *MemoryPoolBuilder::GetDevice() const {
        return device;
    }

    void *MemoryPoolBuilder::GetMemory() const {
        return storage.data();
    }

    u64 MemoryPoolBuilder::GetSize() const {
        return storage.size_bytes();
    }

    MemoryPoolFlags MemoryPoolBuilder::GetFlags() const {
        return flags;
    }

    /* Wrappers */
    void Context::MemoryPoolBuilderSetDefaults(MemoryPoolBuilder *builder) {
        new (builder) MemoryPoolBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, MemoryPoolBuilder, SetDevice, Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, MemoryPoolBuilder, SetStorage, void *, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, MemoryPoolBuilder, SetFlags, MemoryPoolFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, MemoryPoolBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, MemoryPoolBuilder, GetMemory)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, MemoryPoolBuilder, GetSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(MemoryPoolFlags, MemoryPoolBuilder, GetFlags)
    /* End wrappers */

    MemoryPool::MemoryPool(ApiVersion version, VkCore &vkCore, const MemoryPoolBuilder &builder)
        : flags{builder.flags}, vkCore{vkCore}, buffer{vkCore.memoryManager.ImportBuffer(builder.storage)} {
        NNVK_FILL_VERSIONED_STRUCT(MemoryPool);
    }

    MemoryPool::~MemoryPool() {}

    void MemoryPool::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void *MemoryPool::Map() const {
        Logger::Error("Buffer CPU address: 0x{:X}", reinterpret_cast<u64>(buffer.backing.data()));
        return buffer.backing.data();
    }

    void MemoryPool::FlushMappedRange(ptrdiff_t offset, u64 size) const {}

    void MemoryPool::InvalidateMappedRange(ptrdiff_t offset, u64 size) const {}

    BufferAddress MemoryPool::GetBufferAddress() const {
        auto address{vkCore.device.getBufferAddress(vk::BufferDeviceAddressInfo{.buffer = *buffer.vkBuffer})};
        Logger::Error("Buffer GPU address: 0x{:X}", address);
        return address;
    }

    bool MemoryPool::MapVirtual(i32 numRequests, MappingRequest *requests) {
        throw std::runtime_error("MapVirtual is unimplemented");
    }

    u64 MemoryPool::GetSize() const {
        return buffer.backing.size();
    }

    MemoryPoolFlags MemoryPool::GetFlags() const {
        return flags;
    }

    /* Wrappers */
    bool Context::MemoryPoolInitialize(MemoryPool *pool, const MemoryPoolBuilder *builder) {
        new (pool) MemoryPool(apiVersion, vkCore, *builder);
        return true;
    }

    void Context::MemoryPoolFinalize(MemoryPool *pool) {
        pool->~MemoryPool();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, MemoryPool, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(void *, MemoryPool, Map)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, MemoryPool, FlushMappedRange, ptrdiff_t, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, MemoryPool, InvalidateMappedRange, ptrdiff_t, u64)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(BufferAddress, MemoryPool, GetBufferAddress)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, MemoryPool, MapVirtual, i32, MappingRequest *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, MemoryPool, GetSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(MemoryPoolFlags, MemoryPool, GetFlags)
    /* End wrappers */
}