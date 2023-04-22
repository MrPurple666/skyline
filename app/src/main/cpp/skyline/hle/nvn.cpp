// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <range/v3/algorithm/find_if.hpp>
#include <common.h>
#include <nce/guest.h>
#include "nnvk/nnvk.h"
#include "nvn.h"

namespace skyline::hle {
    static void *(*TlsRestorer)(){};

    void SetNvnHleTlsRestorer(void *(*function)()) {
        TlsRestorer = function;
    }

    /**
     * @brief Used to restore the host thread's TLS pointer in functions called from the guest
     */
    class ScopedTlsRestorer {
      private:
        void *tls{};

      public:
        const DeviceState *state;

        ScopedTlsRestorer() : tls{TlsRestorer()}, state{tls ? reinterpret_cast<nce::ThreadContext *>(tls)->state : nullptr} {}

        ~ScopedTlsRestorer() {
            if (tls)
                asm volatile("MSR TPIDR_EL0, %x0"::"r"(tls));
        }
    };

    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }

        char value[N];
    };

    template<StringLiteral Name, typename FuncType, FuncType func>
    struct WrapNvnFunc;

    template<StringLiteral Name, typename Result, typename... Args, Result (nnvk::Context::*func)(Args...)>
    struct WrapNvnFunc<Name, Result (nnvk::Context::*)(Args...), func> {
        static Result GuestCall(Args... args) {
            ScopedTlsRestorer restorer{};

            Logger::Error("Guest called NVN function: {}", Name.value);
            nnvk::Context *ctx{restorer.state->nnvkContext.get()};
            return (ctx->*func)(std::forward<Args>(args)...);
        }
    };

    #define WRAP_NVN_FUNC(name) {#name, reinterpret_cast<void *>(WrapNvnFunc<#name, decltype(&nnvk::Context::name), &nnvk::Context::name>::GuestCall)}
    static const std::array<std::pair<std::string_view, void *>, 216> NvnBootstrapLoaderTable{{
        WRAP_NVN_FUNC(DeviceBuilderSetDefaults),
        WRAP_NVN_FUNC(DeviceBuilderSetFlags),
        WRAP_NVN_FUNC(DeviceBuilderGetFlags),
        WRAP_NVN_FUNC(DeviceInitialize),
        WRAP_NVN_FUNC(DeviceFinalize),
        WRAP_NVN_FUNC(DeviceSetDebugLabel),
        WRAP_NVN_FUNC(DeviceGetInteger),
        WRAP_NVN_FUNC(DeviceGetCurrentTimestampInNanoseconds),
        WRAP_NVN_FUNC(DeviceSetIntermediateShaderCache),
        WRAP_NVN_FUNC(DeviceGetTextureHandle),
        WRAP_NVN_FUNC(DeviceGetTexelFetchHandle),
        WRAP_NVN_FUNC(DeviceGetImageHandle),
        WRAP_NVN_FUNC(DeviceInstallDebugCallback),
        WRAP_NVN_FUNC(DeviceGenerateDebugDomainId),
        WRAP_NVN_FUNC(DeviceSetWindowOriginMode),
        WRAP_NVN_FUNC(DeviceSetDepthMode),
        WRAP_NVN_FUNC(DeviceRegisterFastClearColor),
        WRAP_NVN_FUNC(DeviceRegisterFastClearColori),
        WRAP_NVN_FUNC(DeviceRegisterFastClearColorui),
        WRAP_NVN_FUNC(DeviceRegisterFastClearDepth),
        WRAP_NVN_FUNC(DeviceGetWindowOriginMode),
        WRAP_NVN_FUNC(DeviceGetDepthMode),
        WRAP_NVN_FUNC(DeviceGetTimestampInNanoseconds),
        WRAP_NVN_FUNC(DeviceApplyDeferredFinalizes),
        WRAP_NVN_FUNC(DeviceFinalizeCommandHandle),
        WRAP_NVN_FUNC(DeviceWalkDebugDatabase),
        WRAP_NVN_FUNC(DeviceGetSeparateTextureHandle),
        WRAP_NVN_FUNC(DeviceGetSeparateSamplerHandle),
        WRAP_NVN_FUNC(DeviceIsExternalDebuggerAttached),
        WRAP_NVN_FUNC(MemoryPoolBuilderSetDevice),
        WRAP_NVN_FUNC(MemoryPoolBuilderSetDefaults),
        WRAP_NVN_FUNC(MemoryPoolBuilderSetStorage),
        WRAP_NVN_FUNC(MemoryPoolBuilderSetFlags),
        WRAP_NVN_FUNC(MemoryPoolBuilderGetDevice),
        WRAP_NVN_FUNC(MemoryPoolBuilderGetMemory),
        WRAP_NVN_FUNC(MemoryPoolBuilderGetSize),
        WRAP_NVN_FUNC(MemoryPoolBuilderGetFlags),
        WRAP_NVN_FUNC(MemoryPoolInitialize),
        WRAP_NVN_FUNC(MemoryPoolFinalize),
        WRAP_NVN_FUNC(MemoryPoolSetDebugLabel),
        WRAP_NVN_FUNC(MemoryPoolMap),
        WRAP_NVN_FUNC(MemoryPoolFlushMappedRange),
        WRAP_NVN_FUNC(MemoryPoolInvalidateMappedRange),
        WRAP_NVN_FUNC(MemoryPoolGetBufferAddress),
        WRAP_NVN_FUNC(MemoryPoolMapVirtual),
        WRAP_NVN_FUNC(MemoryPoolGetSize),
        WRAP_NVN_FUNC(MemoryPoolGetFlags),
        WRAP_NVN_FUNC(SamplerPoolInitialize),
        WRAP_NVN_FUNC(SamplerPoolFinalize),
        WRAP_NVN_FUNC(SamplerPoolSetDebugLabel),
        WRAP_NVN_FUNC(SamplerPoolRegisterSampler),
        WRAP_NVN_FUNC(SamplerPoolRegisterSamplerBuilder),
        WRAP_NVN_FUNC(SamplerPoolGetMemoryPool),
        WRAP_NVN_FUNC(SamplerPoolGetMemoryOffset),
        WRAP_NVN_FUNC(SamplerPoolGetSize),
        WRAP_NVN_FUNC(TexturePoolInitialize),
        WRAP_NVN_FUNC(TexturePoolFinalize),
        WRAP_NVN_FUNC(TexturePoolSetDebugLabel),
        WRAP_NVN_FUNC(TexturePoolRegisterTexture),
        WRAP_NVN_FUNC(TexturePoolGetMemoryPool),
        WRAP_NVN_FUNC(TexturePoolGetMemoryOffset),
        WRAP_NVN_FUNC(TexturePoolGetSize),
        WRAP_NVN_FUNC(TextureBuilderSetDevice),
        WRAP_NVN_FUNC(TextureBuilderSetDefaults),
        WRAP_NVN_FUNC(TextureBuilderSetFlags),
        WRAP_NVN_FUNC(TextureBuilderSetTarget),
        WRAP_NVN_FUNC(TextureBuilderSetWidth),
        WRAP_NVN_FUNC(TextureBuilderSetHeight),
        WRAP_NVN_FUNC(TextureBuilderSetDepth),
        WRAP_NVN_FUNC(TextureBuilderSetSize1D),
        WRAP_NVN_FUNC(TextureBuilderSetSize2D),
        WRAP_NVN_FUNC(TextureBuilderSetSize3D),
        WRAP_NVN_FUNC(TextureBuilderSetLevels),
        WRAP_NVN_FUNC(TextureBuilderSetFormat),
        WRAP_NVN_FUNC(TextureBuilderSetSamples),
        WRAP_NVN_FUNC(TextureBuilderSetSwizzle),
        WRAP_NVN_FUNC(TextureBuilderSetDepthStencilMode),
        WRAP_NVN_FUNC(TextureBuilderGetStorageSize),
        WRAP_NVN_FUNC(TextureBuilderGetStorageAlignment),
        WRAP_NVN_FUNC(TextureBuilderSetStorage),
        WRAP_NVN_FUNC(TextureBuilderSetPackagedTextureData),
        WRAP_NVN_FUNC(TextureBuilderSetPackagedTextureLayout),
        WRAP_NVN_FUNC(TextureBuilderSetStride),
        WRAP_NVN_FUNC(TextureBuilderSetGLTextureName),
        WRAP_NVN_FUNC(TextureBuilderGetStorageClass),
        WRAP_NVN_FUNC(TextureBuilderGetDevice),
        WRAP_NVN_FUNC(TextureBuilderGetFlags),
        WRAP_NVN_FUNC(TextureBuilderGetTarget),
        WRAP_NVN_FUNC(TextureBuilderGetWidth),
        WRAP_NVN_FUNC(TextureBuilderGetHeight),
        WRAP_NVN_FUNC(TextureBuilderGetDepth),
        WRAP_NVN_FUNC(TextureBuilderGetLevels),
        WRAP_NVN_FUNC(TextureBuilderGetFormat),
        WRAP_NVN_FUNC(TextureBuilderGetSamples),
        WRAP_NVN_FUNC(TextureBuilderGetSwizzle),
        WRAP_NVN_FUNC(TextureBuilderGetDepthStencilMode),
        WRAP_NVN_FUNC(TextureBuilderGetPackagedTextureData),
        WRAP_NVN_FUNC(TextureBuilderGetPackagedTextureLayout),
        WRAP_NVN_FUNC(TextureBuilderGetStride),
        WRAP_NVN_FUNC(TextureBuilderGetGLTextureName),
        WRAP_NVN_FUNC(TextureBuilderGetZCullStorageSize),
        WRAP_NVN_FUNC(TextureBuilderGetMemoryPool),
        WRAP_NVN_FUNC(TextureBuilderGetMemoryOffset),
        WRAP_NVN_FUNC(TextureViewSetDefaults),
        WRAP_NVN_FUNC(TextureViewSetLevels),
        WRAP_NVN_FUNC(TextureViewSetLayers),
        WRAP_NVN_FUNC(TextureViewSetFormat),
        WRAP_NVN_FUNC(TextureViewSetSwizzle),
        WRAP_NVN_FUNC(TextureViewSetDepthStencilMode),
        WRAP_NVN_FUNC(TextureViewSetTarget),
        WRAP_NVN_FUNC(TextureViewGetLevels),
        WRAP_NVN_FUNC(TextureViewGetLayers),
        WRAP_NVN_FUNC(TextureViewGetFormat),
        WRAP_NVN_FUNC(TextureViewGetSwizzle),
        WRAP_NVN_FUNC(TextureViewGetDepthStencilMode),
        WRAP_NVN_FUNC(TextureViewGetTarget),
        WRAP_NVN_FUNC(TextureInitialize),
        WRAP_NVN_FUNC(TextureFinalize),
        WRAP_NVN_FUNC(TextureGetZCullStorageSize),
        WRAP_NVN_FUNC(TextureSetDebugLabel),
        WRAP_NVN_FUNC(TextureGetStorageClass),
        WRAP_NVN_FUNC(TextureGetViewOffset),
        WRAP_NVN_FUNC(TextureGetFlags),
        WRAP_NVN_FUNC(TextureGetTarget),
        WRAP_NVN_FUNC(TextureGetWidth),
        WRAP_NVN_FUNC(TextureGetHeight),
        WRAP_NVN_FUNC(TextureGetDepth),
        WRAP_NVN_FUNC(TextureGetLevels),
        WRAP_NVN_FUNC(TextureGetFormat),
        WRAP_NVN_FUNC(TextureGetSamples),
        WRAP_NVN_FUNC(TextureGetSwizzle),
        WRAP_NVN_FUNC(TextureGetDepthStencilMode),
        WRAP_NVN_FUNC(TextureGetStride),
        WRAP_NVN_FUNC(TextureGetTextureAddress),
        WRAP_NVN_FUNC(TextureWriteTexels),
        WRAP_NVN_FUNC(TextureWriteTexelsStrided),
        WRAP_NVN_FUNC(TextureReadTexels),
        WRAP_NVN_FUNC(TextureReadTexelsStrided),
        WRAP_NVN_FUNC(TextureFlushTexels),
        WRAP_NVN_FUNC(TextureInvalidateTexels),
        WRAP_NVN_FUNC(TextureGetMemoryPool),
        WRAP_NVN_FUNC(TextureGetMemoryOffset),
        WRAP_NVN_FUNC(TextureGetStorageSize),
        WRAP_NVN_FUNC(TextureCompare),
        WRAP_NVN_FUNC(TextureGetDebugID),
        WRAP_NVN_FUNC(SamplerBuilderSetDefaults),
        WRAP_NVN_FUNC(SamplerBuilderSetDevice),
        WRAP_NVN_FUNC(SamplerBuilderSetMinMagFilter),
        WRAP_NVN_FUNC(SamplerBuilderSetWrapMode),
        WRAP_NVN_FUNC(SamplerBuilderSetLodClamp),
        WRAP_NVN_FUNC(SamplerBuilderSetLodBias),
        WRAP_NVN_FUNC(SamplerBuilderSetCompare),
        WRAP_NVN_FUNC(SamplerBuilderSetBorderColor),
        WRAP_NVN_FUNC(SamplerBuilderSetBorderColori),
        WRAP_NVN_FUNC(SamplerBuilderSetBorderColorui),
        WRAP_NVN_FUNC(SamplerBuilderSetMaxAnisotropy),
        WRAP_NVN_FUNC(SamplerBuilderSetReductionFilter),
        WRAP_NVN_FUNC(SamplerBuilderSetLodSnap),
        WRAP_NVN_FUNC(SamplerBuilderGetDevice),
        WRAP_NVN_FUNC(SamplerBuilderGetMinMagFilter),
        WRAP_NVN_FUNC(SamplerBuilderGetWrapMode),
        WRAP_NVN_FUNC(SamplerBuilderGetLodClamp),
        WRAP_NVN_FUNC(SamplerBuilderGetLodBias),
        WRAP_NVN_FUNC(SamplerBuilderGetCompare),
        WRAP_NVN_FUNC(SamplerBuilderGetBorderColor),
        WRAP_NVN_FUNC(SamplerBuilderGetBorderColori),
        WRAP_NVN_FUNC(SamplerBuilderGetBorderColorui),
        WRAP_NVN_FUNC(SamplerBuilderGetMaxAnisotropy),
        WRAP_NVN_FUNC(SamplerBuilderGetReductionFilter),
        WRAP_NVN_FUNC(SamplerBuilderGetLodSnap),
        WRAP_NVN_FUNC(SamplerInitialize),
        WRAP_NVN_FUNC(SamplerFinalize),
        WRAP_NVN_FUNC(SamplerSetDebugLabel),
        WRAP_NVN_FUNC(SamplerGetMinMagFilter),
        WRAP_NVN_FUNC(SamplerGetWrapMode),
        WRAP_NVN_FUNC(SamplerGetLodClamp),
        WRAP_NVN_FUNC(SamplerGetLodBias),
        WRAP_NVN_FUNC(SamplerGetCompare),
        WRAP_NVN_FUNC(SamplerGetBorderColor),
        WRAP_NVN_FUNC(SamplerGetBorderColori),
        WRAP_NVN_FUNC(SamplerGetBorderColorui),
        WRAP_NVN_FUNC(SamplerGetMaxAnisotropy),
        WRAP_NVN_FUNC(SamplerGetReductionFilter),
        WRAP_NVN_FUNC(SamplerCompare),
        WRAP_NVN_FUNC(SamplerGetDebugID),
        WRAP_NVN_FUNC(QueueBuilderSetDefaults),
        WRAP_NVN_FUNC(QueueBuilderSetDevice),
        WRAP_NVN_FUNC(QueueBuilderSetFlags),
        WRAP_NVN_FUNC(QueueBuilderSetCommandMemorySize),
        WRAP_NVN_FUNC(QueueBuilderSetComputeMemorySize),
        WRAP_NVN_FUNC(QueueBuilderSetControlMemorySize),
        WRAP_NVN_FUNC(QueueBuilderGetQueueMemorySize),
        WRAP_NVN_FUNC(QueueBuilderSetQueueMemory),
        WRAP_NVN_FUNC(QueueBuilderSetCommandFlushThreshold),
        WRAP_NVN_FUNC(QueueBuilderGetDevice),
        WRAP_NVN_FUNC(QueueBuilderGetFlags),
        WRAP_NVN_FUNC(QueueBuilderGetCommandMemorySize),
        WRAP_NVN_FUNC(QueueBuilderGetComputeMemorySize),
        WRAP_NVN_FUNC(QueueBuilderGetControlMemorySize),
        WRAP_NVN_FUNC(QueueBuilderGetCommandFlushThreshold),
        WRAP_NVN_FUNC(QueueBuilderGetMemorySize),
        WRAP_NVN_FUNC(QueueBuilderGetMemory),
        WRAP_NVN_FUNC(QueueInitialize),
        WRAP_NVN_FUNC(QueueFinalize),
        WRAP_NVN_FUNC(QueueGetError),
        WRAP_NVN_FUNC(QueueGetTotalCommandMemoryUsed),
        WRAP_NVN_FUNC(QueueGetTotalControlMemoryUsed),
        WRAP_NVN_FUNC(QueueGetTotalComputeMemoryUsed),
        WRAP_NVN_FUNC(QueueResetMemoryUsageCounts),
        WRAP_NVN_FUNC(QueueSetDebugLabel),
        WRAP_NVN_FUNC(QueueSubmitCommands),
        WRAP_NVN_FUNC(QueueFlush),
        WRAP_NVN_FUNC(QueueFinish),
        WRAP_NVN_FUNC(QueuePresentTexture),
        WRAP_NVN_FUNC(QueueFenceSync),
        WRAP_NVN_FUNC(QueueWaitSync),
    }};
    #undef WRAP_NVN_FUNC

    /**
     * @note This is effectively the same as nvnBootstrapLoader but since it can be called from the guest we need to wrap it
     */
    static void *NvnDeviceGetProcAddress(void *device, const char *name) {
        ScopedTlsRestorer restorer{};
        return NvnBootstrapLoader(name);
    }

    void *NvnBootstrapLoader(const char *name) {
        if (!strncmp(name, "nvnDeviceGetProcAddress", 23))
            return reinterpret_cast<void *>(NvnDeviceGetProcAddress);

        auto it{ranges::find_if(NvnBootstrapLoaderTable, [&](const auto &entry) {
            return entry.first == (name + 3);
        })};

        if (it == NvnBootstrapLoaderTable.end()) {
            static thread_local size_t count{};
            count += sizeof(u32);
            Logger::Error("Unimplemented NVN function: {} 0x{:X}", name, count);
            return reinterpret_cast<void *>(count);
        }

        return it->second;
    }
}
