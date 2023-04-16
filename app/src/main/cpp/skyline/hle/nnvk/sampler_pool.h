// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "types.h"
#include "versioning.h"

namespace nnvk {
    class MemoryPool;

    class Sampler;

    class SamplerBuilder;

    class SamplerPool {
      private:
        const char *debugLabel{};
        MemoryPool *memoryPool;
        i64 memoryOffset;
        i32 numDescriptors;

      public:
        SamplerPool(ApiVersion version, MemoryPool *memoryPool, i64 memoryPoolOffset, i32 numDescriptors);

        ~SamplerPool();

        void SetDebugLabel(const char *label);

        void RegisterSampler(i32 id, const Sampler *sampler);

        void RegisterSamplerBuilder(i32 id, const SamplerBuilder *builder);

        const MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;

        i32 GetSize() const;
    };
    NNVK_VERSIONED_STRUCT(SamplerPool, 0x20);

}
