// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vulkan/vulkan_core.h>
#include "types.h"
#include "vkcore.h"
#include "memory_manager.h"
#include "versioning.h"
#include "device.h"
#include "memory_pool.h"
#include "sampler_pool.h"
#include "texture_pool.h"
#include "texture.h"
#include "sampler.h"
#include "queue.h"

#define NNVK_CONTEXT_WRAP_TRIVIAL_0(return_type, type, method) \
    return_type Context::type ## method(type *name) { \
        return name->method(); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_1(return_type, type, method, arg0_type) \
    return_type Context::type ## method(type *name, arg0_type arg0) { \
        return name->method(arg0); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_2(return_type, type, method, arg0_type, arg1_type) \
    return_type Context::type ## method(type *name, arg0_type arg0, arg1_type arg1) { \
        return name->method(arg0, arg1); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_3(return_type, type, method, arg0_type, arg1_type, arg2_type) \
    return_type Context::type ## method(type *name, arg0_type arg0, arg1_type arg1, arg2_type arg2) { \
        return name->method(arg0, arg1, arg2); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_4(return_type, type, method, arg0_type, arg1_type, arg2_type, arg3_type) \
   return_type Context::type ## method(type *name, arg0_type arg0, arg1_type arg1, arg2_type arg2, arg3_type arg3) { \
        return name->method(arg0, arg1, arg2, arg3); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_5(return_type, type, method, arg0_type, arg1_type, arg2_type, arg3_type, arg4_type) \
    return_type Context::type ## method(type *name, arg0_type arg0, arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4) { \
        return name->method(arg0, arg1, arg2, arg3, arg4); \
    }

#define NNVK_CONTEXT_WRAP_TRIVIAL_6(return_type, type, method, arg0_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
    return_type Context::type ## method(type *name, arg0_type arg0, arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5) { \
        return name->method(arg0, arg1, arg2, arg3, arg4, arg5); \
    }
namespace nnvk {
    class Context {
      private:
        ApiVersion apiVersion{"0.0"_V};
        VkCore vkCore;

      public:
        Context(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, void *adrenotoolsImportHandle, bool enableValidation);

        void SetApiVersion(i32 major, i32 minor);

        void DeviceBuilderSetDefaults(DeviceBuilder *builder);

        void DeviceBuilderSetFlags(DeviceBuilder *builder, DeviceFlags flags);

        DeviceFlags DeviceBuilderGetFlags(DeviceBuilder *builder);

        bool DeviceInitialize(Device *device, const DeviceBuilder *builder);

        void DeviceFinalize(Device *device);

        void DeviceSetDebugLabel(Device *device, const char *label);

        void DeviceGetInteger(Device *device, DeviceInfo pname, i32 *v);

        u64 DeviceGetCurrentTimestampInNanoseconds(Device *device);

        void DeviceSetIntermediateShaderCache(Device *device, i32 i);

        TextureHandle DeviceGetTextureHandle(Device *device, i32 textureId, i32 samplerId);

        TextureHandle DeviceGetTexelFetchHandle(Device *device, i32 textureId);

        ImageHandle DeviceGetImageHandle(Device *device, i32 textureId);

        void DeviceInstallDebugCallback(Device *device, const void *callback, void *callbackData, bool enable);

        DebugDomainId DeviceGenerateDebugDomainId(Device *device, const char *s);

        void DeviceSetWindowOriginMode(Device *device, WindowOriginMode mode);

        void DeviceSetDepthMode(Device *device, DepthMode mode);

        bool DeviceRegisterFastClearColor(Device *device, const f32 *color, Format format);

        bool DeviceRegisterFastClearColori(Device *device, const i32 *color, Format format);

        bool DeviceRegisterFastClearColorui(Device *device, const u32 *color, Format format);

        bool DeviceRegisterFastClearDepth(Device *device, f32 f);

        WindowOriginMode DeviceGetWindowOriginMode(Device *device);

        DepthMode DeviceGetDepthMode(Device *device);

        u64 DeviceGetTimestampInNanoseconds(Device *device, const CounterData *counterData);

        void DeviceApplyDeferredFinalizes(Device *device, i32 i);

        void DeviceFinalizeCommandHandle(Device *device, CommandHandle handles);

        void DeviceWalkDebugDatabase(Device *device, DebugObjectType type, const void *callback, void *callbackData);

        SeparateTextureHandle DeviceGetSeparateTextureHandle(Device *device, i32 textureId);

        SeparateSamplerHandle DeviceGetSeparateSamplerHandle(Device *device, i32 samplerId);

        bool DeviceIsExternalDebuggerAttached(Device *device);

        void MemoryPoolBuilderSetDevice(MemoryPoolBuilder *builder, Device *device);

        void MemoryPoolBuilderSetDefaults(MemoryPoolBuilder *builder);

        void MemoryPoolBuilderSetStorage(MemoryPoolBuilder *builder, void *storage, u64 size);

        void MemoryPoolBuilderSetFlags(MemoryPoolBuilder *builder, MemoryPoolFlags flags);

        const Device *MemoryPoolBuilderGetDevice(MemoryPoolBuilder *builder);

        void *MemoryPoolBuilderGetMemory(MemoryPoolBuilder *builder);

        u64 MemoryPoolBuilderGetSize(MemoryPoolBuilder *builder);

        MemoryPoolFlags MemoryPoolBuilderGetFlags(MemoryPoolBuilder *builder);

        bool MemoryPoolInitialize(MemoryPool *pool, const MemoryPoolBuilder *builder);

        void MemoryPoolFinalize(MemoryPool *pool);

        void MemoryPoolSetDebugLabel(MemoryPool *pool, const char *label);

        void *MemoryPoolMap(MemoryPool *pool);

        void MemoryPoolFlushMappedRange(MemoryPool *pool, ptrdiff_t offset, u64 size);

        void MemoryPoolInvalidateMappedRange(MemoryPool *pool, ptrdiff_t offset, u64 size);

        BufferAddress MemoryPoolGetBufferAddress(MemoryPool *pool);

        bool MemoryPoolMapVirtual(MemoryPool *pool, i32 numRequests, MappingRequest *requests);

        u64 MemoryPoolGetSize(MemoryPool *pool);

        MemoryPoolFlags MemoryPoolGetFlags(MemoryPool *pool);

        bool SamplerPoolInitialize(SamplerPool *pool, MemoryPool *memoryPool, i64 offset, i32 numDescriptors);

        void SamplerPoolFinalize(SamplerPool *pool);

        void SamplerPoolSetDebugLabel(SamplerPool *pool, const char *label);

        void SamplerPoolRegisterSampler(SamplerPool *pool, i32 index, const Sampler *sampler);

        void SamplerPoolRegisterSamplerBuilder(SamplerPool *pool, i32 index, const SamplerBuilder *sampler);

        const MemoryPool *SamplerPoolGetMemoryPool(SamplerPool *pool);

        i64 SamplerPoolGetMemoryOffset(SamplerPool *pool);

        i32 SamplerPoolGetSize(SamplerPool *pool);

        bool TexturePoolInitialize(TexturePool *pool, MemoryPool *memoryPool, i64 offset, i32 numDescriptors);

        void TexturePoolFinalize(TexturePool *pool);

        void TexturePoolSetDebugLabel(TexturePool *pool, const char *label);

        void TexturePoolRegisterTexture(TexturePool *pool, i32 index, const Texture *sampler);

        const MemoryPool *TexturePoolGetMemoryPool(TexturePool *pool);

        i64 TexturePoolGetMemoryOffset(TexturePool *pool);

        i32 TexturePoolGetSize(TexturePool *pool);
        
        void TextureBuilderSetDevice(TextureBuilder *builder, Device *device);

        void TextureBuilderSetDefaults(TextureBuilder *builder);

        void TextureBuilderSetFlags(TextureBuilder *builder, TextureFlags flags);

        void TextureBuilderSetTarget(TextureBuilder *builder, TextureTarget target);

        void TextureBuilderSetWidth(TextureBuilder *builder, i32 width);

        void TextureBuilderSetHeight(TextureBuilder *builder, i32 height);

        void TextureBuilderSetDepth(TextureBuilder *builder, i32 depth);

        void TextureBuilderSetSize1D(TextureBuilder *builder, i32 width);

        void TextureBuilderSetSize2D(TextureBuilder *builder, i32 width, i32 height);

        void TextureBuilderSetSize3D(TextureBuilder *builder, i32 width, i32 height, i32 depth);

        void TextureBuilderSetLevels(TextureBuilder *builder, i32 numLevels);

        void TextureBuilderSetFormat(TextureBuilder *builder, Format format);

        void TextureBuilderSetSamples(TextureBuilder *builder, i32 samples);

        void TextureBuilderSetSwizzle(TextureBuilder *builder, TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a);

        void TextureBuilderSetDepthStencilMode(TextureBuilder *builder, TextureDepthStencilMode mode);

        size_t TextureBuilderGetStorageSize(TextureBuilder *builder);

        size_t TextureBuilderGetStorageAlignment(TextureBuilder *builder);

        void TextureBuilderSetStorage(TextureBuilder *builder, MemoryPool *pool, i64 offset);

        void TextureBuilderSetPackagedTextureData(TextureBuilder *builder, const void *data);

        void TextureBuilderSetPackagedTextureLayout(TextureBuilder *builder, const PackagedTextureLayout *layout);

        void TextureBuilderSetStride(TextureBuilder *builder, i64 stride);

        void TextureBuilderSetGLTextureName(TextureBuilder *builder, u32 name);

        StorageClass TextureBuilderGetStorageClass(TextureBuilder *builder);

        const Device *TextureBuilderGetDevice(TextureBuilder *builder);

        TextureFlags TextureBuilderGetFlags(TextureBuilder *builder);

        TextureTarget TextureBuilderGetTarget(TextureBuilder *builder);

        i32 TextureBuilderGetWidth(TextureBuilder *builder);

        i32 TextureBuilderGetHeight(TextureBuilder *builder);

        i32 TextureBuilderGetDepth(TextureBuilder *builder);

        i32 TextureBuilderGetLevels(TextureBuilder *builder);

        Format TextureBuilderGetFormat(TextureBuilder *builder);

        i32 TextureBuilderGetSamples(TextureBuilder *builder);

        void TextureBuilderGetSwizzle(TextureBuilder *builder, TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a);

        TextureDepthStencilMode TextureBuilderGetDepthStencilMode(TextureBuilder *builder);

        const void *TextureBuilderGetPackagedTextureData(TextureBuilder *builder);

        bool TextureBuilderGetPackagedTextureLayout(TextureBuilder *builder, PackagedTextureLayout *layout);

        i64 TextureBuilderGetStride(TextureBuilder *builder);

 //       void TextureBuilderGetSparseTileLayout(TextureBuilder *builder, TextureSparseTileLayout *layout);

        u32 TextureBuilderGetGLTextureName(TextureBuilder *builder);

        u64 TextureBuilderGetZCullStorageSize(TextureBuilder *builder);

        const MemoryPool *TextureBuilderGetMemoryPool(TextureBuilder *builder);

        i64 TextureBuilderGetMemoryOffset(TextureBuilder *builder);

        void TextureViewSetDefaults(TextureView *view);

        void TextureViewSetLevels(TextureView *view, i32 baseLevel, i32 numLevels);

        void TextureViewSetLayers(TextureView *view, i32 minLayer, i32 numLayers);

        void TextureViewSetFormat(TextureView *view, Format format);

        void TextureViewSetSwizzle(TextureView *view, TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a);

        void TextureViewSetDepthStencilMode(TextureView *view, TextureDepthStencilMode mode);

        void TextureViewSetTarget(TextureView *view, TextureTarget target);

        bool TextureViewGetLevels(TextureView *view, i32 *baseLevel, i32 *numLevels);

        bool TextureViewGetLayers(TextureView *view, i32 *minLayer, i32 *numLayers);

        bool TextureViewGetFormat(TextureView *view, Format *format);

        bool TextureViewGetSwizzle(TextureView *view, TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a);

        bool TextureViewGetDepthStencilMode(TextureView *view, TextureDepthStencilMode *mode);

        bool TextureViewGetTarget(TextureView *view, TextureTarget *target);

        bool TextureInitialize(Texture *texture, const TextureBuilder *builder);

        void TextureFinalize(Texture *texture);

        size_t TextureGetZCullStorageSize(Texture *texture);

        void TextureSetDebugLabel(Texture *texture, const char *label);

        StorageClass TextureGetStorageClass(Texture *texture);

        i64 TextureGetViewOffset(Texture *texture, TextureView *view);

        TextureFlags TextureGetFlags(Texture *texture);

        TextureTarget TextureGetTarget(Texture *texture);

        i32 TextureGetWidth(Texture *texture);

        i32 TextureGetHeight(Texture *texture);

        i32 TextureGetDepth(Texture *texture);

        i32 TextureGetLevels(Texture *texture);

        Format TextureGetFormat(Texture *texture);

        i32 TextureGetSamples(Texture *texture);

        void TextureGetSwizzle(Texture *texture, TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a);

        TextureDepthStencilMode TextureGetDepthStencilMode(Texture *texture);

        i64 TextureGetStride(Texture *texture);

        TextureAddress TextureGetTextureAddress(Texture *texture);

        void TextureWriteTexels(Texture *texture, const TextureView *view, const CopyRegion *region, const void *p);

        void TextureWriteTexelsStrided(Texture *texture, const TextureView *view, const CopyRegion *region, const void *p, i64 o1, i64 o2);

        void TextureReadTexels(Texture *texture, const TextureView *view, const CopyRegion *region, void *p);

        void TextureReadTexelsStrided(Texture *texture, const TextureView *view, const CopyRegion *region, void *p, i64 o1, i64 o2);

        void TextureFlushTexels(Texture *texture, const TextureView *view, const CopyRegion *region);

        void TextureInvalidateTexels(Texture *texture, const TextureView *view, const CopyRegion *region);

        MemoryPool *TextureGetMemoryPool(Texture *texture);

        i64 TextureGetMemoryOffset(Texture *texture);

        i32 TextureGetStorageSize(Texture *texture);

        bool TextureCompare(Texture *texture, const Texture *texture2);

        u64 TextureGetDebugID(Texture *texture);

        void SamplerBuilderSetDefaults(SamplerBuilder *builder);

        void SamplerBuilderSetDevice(SamplerBuilder *builder, const Device *device);

        void SamplerBuilderSetMinMagFilter(SamplerBuilder *builder, MinFilter min, MagFilter mag);

        void SamplerBuilderSetWrapMode(SamplerBuilder *builder, WrapMode s, WrapMode t, WrapMode r);

        void SamplerBuilderSetLodClamp(SamplerBuilder *builder, f32 min, f32 max);

        void SamplerBuilderSetLodBias(SamplerBuilder *builder, f32 bias);

        void SamplerBuilderSetCompare(SamplerBuilder *builder, CompareMode mode, CompareFunc func);

        void SamplerBuilderSetBorderColor(SamplerBuilder *builder, const f32 *borderColor);

        void SamplerBuilderSetBorderColori(SamplerBuilder *builder, const i32 *borderColor);

        void SamplerBuilderSetBorderColorui(SamplerBuilder *builder, const u32 *borderColor);

        void SamplerBuilderSetMaxAnisotropy(SamplerBuilder *builder, f32 maxAniso);

        void SamplerBuilderSetReductionFilter(SamplerBuilder *builder, SamplerReduction filter);

        void SamplerBuilderSetLodSnap(SamplerBuilder *builder, f32 f);

        const Device *SamplerBuilderGetDevice(SamplerBuilder *builder);

        void SamplerBuilderGetMinMagFilter(SamplerBuilder *builder, MinFilter *min, MagFilter *mag);

        void SamplerBuilderGetWrapMode(SamplerBuilder *builder, WrapMode *s, WrapMode *t, WrapMode *r);

        void SamplerBuilderGetLodClamp(SamplerBuilder *builder, f32 *min, f32 *max);

        f32 SamplerBuilderGetLodBias(SamplerBuilder *builder);

        void SamplerBuilderGetCompare(SamplerBuilder *builder, CompareMode *mode, CompareFunc *func);

        void SamplerBuilderGetBorderColor(SamplerBuilder *builder, f32 *borderColor);

        void SamplerBuilderGetBorderColori(SamplerBuilder *builder, i32 *borderColor);

        void SamplerBuilderGetBorderColorui(SamplerBuilder *builder, u32 *borderColor);

        f32 SamplerBuilderGetMaxAnisotropy(SamplerBuilder *builder);

        SamplerReduction SamplerBuilderGetReductionFilter(SamplerBuilder *builder);

        f32 SamplerBuilderGetLodSnap(SamplerBuilder *builder);

        bool SamplerInitialize(Sampler *sampler, const SamplerBuilder *builder);

        void SamplerFinalize(Sampler *sampler);

        void SamplerSetDebugLabel(Sampler *sampler, const char *label);

        void SamplerGetMinMagFilter(Sampler *sampler, MinFilter *min, MagFilter *mag);

        void SamplerGetWrapMode(Sampler *sampler, WrapMode *s, WrapMode *t, WrapMode *r);

        void SamplerGetLodClamp(Sampler *sampler, f32 *min, f32 *max);

        f32 SamplerGetLodBias(Sampler *sampler);

        void SamplerGetCompare(Sampler *sampler, CompareMode *mode, CompareFunc *func);

        void SamplerGetBorderColor(Sampler *sampler, f32 *borderColor);

        void SamplerGetBorderColori(Sampler *sampler, i32 *borderColor);

        void SamplerGetBorderColorui(Sampler *sampler, u32 *borderColor);

        f32 SamplerGetMaxAnisotropy(Sampler *sampler);

        SamplerReduction SamplerGetReductionFilter(Sampler *sampler);

        bool SamplerCompare(Sampler *sampler, const Sampler *sampler2);

        u64 SamplerGetDebugID(Sampler *sampler);

        void QueueBuilderSetDefaults(QueueBuilder *builder);

        void QueueBuilderSetDevice(QueueBuilder *builder, Device *device);

        void QueueBuilderSetFlags(QueueBuilder *builder, QueueFlags flags);

        void QueueBuilderSetCommandMemorySize(QueueBuilder *builder, u64 size);

        void QueueBuilderSetComputeMemorySize(QueueBuilder *builder, u64 size);

        void QueueBuilderSetControlMemorySize(QueueBuilder *builder, u64 size);

        u64 QueueBuilderGetQueueMemorySize(QueueBuilder *builder);

        void QueueBuilderSetQueueMemory(QueueBuilder *builder, void *memory, u64 size);

        void QueueBuilderSetCommandFlushThreshold(QueueBuilder *builder, u64 threshold);

        const Device *QueueBuilderGetDevice(QueueBuilder *builder);

        QueueFlags QueueBuilderGetFlags(QueueBuilder *builder);

        bool QueueBuilderGetCommandMemorySize(QueueBuilder *builder, i32 *i);

        bool QueueBuilderGetComputeMemorySize(QueueBuilder *builder, i32 *i);

        bool QueueBuilderGetControlMemorySize(QueueBuilder *builder, i32 *i);

        bool QueueBuilderGetCommandFlushThreshold(QueueBuilder *builder, i32 *i);

        int QueueBuilderGetMemorySize(QueueBuilder *builder);

        void *QueueBuilderGetMemory(QueueBuilder *builder);

        bool QueueInitialize(Queue *queue, const QueueBuilder *builder);

        void QueueFinalize(Queue *queue);

        QueueGetErrorResult QueueGetError(Queue *queue, QueueErrorInfo *errorInfo);

        u64 QueueGetTotalCommandMemoryUsed(Queue *queue);

        u64 QueueGetTotalControlMemoryUsed(Queue *queue);

        u64 QueueGetTotalComputeMemoryUsed(Queue *queue);

        void QueueResetMemoryUsageCounts(Queue *queue);

        void QueueSetDebugLabel(Queue *queue, const char *label);

        void QueueSubmitCommands(Queue *queue, i32 numCommands, const CommandHandle *handles);

        void QueueFlush(Queue *queue);

        void QueueFinish(Queue *queue);

        void QueuePresentTexture(Queue *queue, Window *window, i32 *textureIndex);

        void QueueFenceSync(Queue *queue, Sync *sync, SyncCondition condition, SyncFlags flags);

        bool QueueWaitSync(Queue *queue, Sync *sync);
    };
}
