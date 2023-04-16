#include <new>
#include <stdexcept>
#include "nnvk.h"
#include "logging.h"
#include "texture_pool.h"

namespace nnvk {
    TexturePool::TexturePool(ApiVersion version, MemoryPool *memoryPool, i64 memoryPoolOffset, i32 numDescriptors)
        : memoryPool{memoryPool}, memoryOffset{memoryPoolOffset}, numDescriptors{numDescriptors} {
        NNVK_FILL_VERSIONED_STRUCT(TexturePool);
    }

    TexturePool::~TexturePool() {}

    void TexturePool::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void TexturePool::RegisterTexture(i32 id, const Texture *sampler) {
        Logger::Error("TexturePool::RegisterTexture is not implemented");
    }

    const MemoryPool *TexturePool::GetMemoryPool() const {
        return memoryPool;
    }

    i64 TexturePool::GetMemoryOffset() const {
        return memoryOffset;
    }

    i32 TexturePool::GetSize() const {
        // TODO VALIDATE
        return numDescriptors;
    }


    /* Wrappers */
    bool Context::TexturePoolInitialize(TexturePool *pool, MemoryPool *memoryPool, i64 offset, i32 numDescriptors) {
        new (pool) TexturePool(apiVersion, memoryPool, offset, numDescriptors);
        return true;
    }

    void Context::TexturePoolFinalize(TexturePool *pool) {
        pool->~TexturePool();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TexturePool, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, TexturePool, RegisterTexture, i32, const Texture *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const MemoryPool *, TexturePool, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, TexturePool, GetMemoryOffset)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TexturePool, GetSize)
    /* End wrappers */
}