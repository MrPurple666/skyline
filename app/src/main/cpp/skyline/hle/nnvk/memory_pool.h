// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "memory_manager.h"
#include "types.h"
#include "versioning.h"

namespace nnvk {
    struct VkCore;

    struct MemoryPoolFlags {
        bool cpuNoAccess : 1;
        bool cpuUncached : 1;
        bool gpuNoAccess : 1;
        bool gpuUncached : 1;
        bool gpuCached : 1;
        bool shaderCode : 1;
        bool compressible : 1;
        bool physical : 1;
        bool isVirtual : 1;
        u32 _pad_ : 23;
    };
    static_assert(sizeof(MemoryPoolFlags) == 4);

    class Device;
    class MemoryPool;

    class MemoryPoolBuilder {
      private:
        friend MemoryPool;

        Device *device{};
        std::span<u8> storage;
        MemoryPoolFlags flags{};
        u32 _pad_{};

      public:
        MemoryPoolBuilder(ApiVersion version);

        void SetDevice(Device *device);

        void SetStorage(void *memory, u64 size);

        void SetFlags(MemoryPoolFlags newFlags);

        const Device *GetDevice() const;

        void *GetMemory() const;

        u64 GetSize() const;

        MemoryPoolFlags GetFlags() const;
    };
    NNVK_VERSIONED_STRUCT(MemoryPoolBuilder, 0x40);

    using BufferAddress = u64;
    using MappingRequest = void *;

    class MemoryPool {
      private:
        const char *debugLabel{};
        MemoryPoolFlags flags;
        VkCore &vkCore;
        vkcore::ImportedBuffer buffer;

      public:
        MemoryPool(ApiVersion version, VkCore &vkCore, const MemoryPoolBuilder &builder);

        ~MemoryPool();

        void SetDebugLabel(const char *label);

        void *Map() const;

        void FlushMappedRange(ptrdiff_t offset, u64 size) const;

        void InvalidateMappedRange(ptrdiff_t offset, u64 size) const;

        BufferAddress GetBufferAddress() const;

        bool MapVirtual(i32 numRequests, MappingRequest *requests);

        u64 GetSize() const;

        MemoryPoolFlags GetFlags() const;
    };
    NNVK_VERSIONED_STRUCT(MemoryPool, 0x100);
}
