#include <new>
#include <stdexcept>
#include "nnvk.h"
#include "sampler_pool.h"

namespace nnvk {
    SamplerPool::SamplerPool(ApiVersion version, MemoryPool *memoryPool, i64 memoryPoolOffset, i32 numDescriptors)
        : memoryPool{memoryPool}, memoryOffset{memoryPoolOffset}, numDescriptors{numDescriptors} {
        NNVK_FILL_VERSIONED_STRUCT(SamplerPool);
    }

    SamplerPool::~SamplerPool() {}

    void SamplerPool::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void SamplerPool::RegisterSampler(i32 id, const Sampler *sampler) {
        throw std::runtime_error("SamplerPool::RegisterSampler is not implemented");
    }

    void SamplerPool::RegisterSamplerBuilder(i32 id, const SamplerBuilder *builder) {
        throw std::runtime_error("SamplerPool::RegisterSamplerBuilder is not implemented");
    }

    const MemoryPool *SamplerPool::GetMemoryPool() const {
        return memoryPool;
    }

    i64 SamplerPool::GetMemoryOffset() const {
        return memoryOffset;
    }

    i32 SamplerPool::GetSize() const {
        // TODO VALIDATE
        return numDescriptors;
    }


    /* Wrappers */
    bool Context::SamplerPoolInitialize(SamplerPool *pool, MemoryPool *memoryPool, i64 offset, i32 numDescriptors) {
        new (pool) SamplerPool(apiVersion, memoryPool, offset, numDescriptors);
        return true;
    }

    void Context::SamplerPoolFinalize(SamplerPool *pool) {
        pool->~SamplerPool();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerPool, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerPool, RegisterSampler, i32, const Sampler *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerPool, RegisterSamplerBuilder, i32, const SamplerBuilder *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const MemoryPool *, SamplerPool, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, SamplerPool, GetMemoryOffset)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, SamplerPool, GetSize)
    /* End wrappers */
}