// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "types.h"
#include "versioning.h"

namespace nnvk {
    class MemoryPool;

    class Texture;

    class TextureBuilder;

    class TexturePool {
      private:
        const char *debugLabel{};
        MemoryPool *memoryPool;
        i64 memoryOffset;
        i32 numDescriptors;

      public:
        TexturePool(ApiVersion version, MemoryPool *memoryPool, i64 memoryPoolOffset, i32 numDescriptors);

        ~TexturePool();

        void SetDebugLabel(const char *label);

        void RegisterTexture(i32 id, const Texture *sampler);

        const MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;

        i32 GetSize() const;
    };
    NNVK_VERSIONED_STRUCT(TexturePool, 0x20);
}
