// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "types.h"
#include "versioning.h"
#include "memory_pool.h"

namespace nnvk {
    class Device;
    class Buffer;

    class BufferBuilder {
      private:
        friend Buffer;

        Device *device{};
        MemoryPool *memoryPool{};
        i64 memoryOffset{};
        u64 memorySize{};

      public:
        BufferBuilder(ApiVersion version);

        void SetDevice(Device *device);

        void SetStorage(MemoryPool *pool, i64 offset, u64 size);

        const Device *GetDevice() const;

        const MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;

        u64 GetSize() const;
    };
    NNVK_VERSIONED_STRUCT(BufferBuilder, 0x40);

    class Buffer {
      private:
        const char *debugLabel{};
        MemoryPool *memoryPool;
        i64 memoryOffset;
        u64 memorySize;

      public:
        Buffer(ApiVersion version, const BufferBuilder  &builder);

        ~Buffer();

        void SetDebugLabel(const char *label);

        void *Map() const;

        BufferAddress GetAddress() const;

        void FlushMappedRange(i64 offset, u64 size) const;

        void InvalidateMappedRange(i64 offset, u64 size) const;

        MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;

        u64 GetSize() const;

        u64 GetDebugID() const;
    };
    NNVK_VERSIONED_STRUCT(Buffer, 0x30);
}
