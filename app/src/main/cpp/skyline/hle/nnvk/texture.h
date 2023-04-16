// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <optional>
#include <utility>
#include <vulkan/vulkan_raii.hpp>
#include "types.h"
#include "versioning.h"
#include "format.h"

namespace nnvk {
    class Device;

    struct TextureFlags {
        bool display : 1;
        bool videoDecode : 1;
        bool image : 1;
        bool compressible : 1;
        bool linear : 1;
        bool sparse : 1;
        bool linearRenderTarget : 1;
        bool adaptiveZCull : 1;
        bool depth16PreferFastClear : 1;
        bool minimalLayout : 1;
        bool zCullSupportStencil : 1;
        u32 _pad_ : 21;
    };
    static_assert(sizeof(TextureFlags) == 0x4);

    enum class TextureTarget : i32 {
        Target1D,
        Target2D,
        Target3D,
        Target1DArray,
        Target2DArray,
        Target2DMultisample,
        Target2DMultisampleArray,
        TargetRectangle,
        TargetCubemap,
        TargetCubemapArray,
        TargetBuffer,
    };

    enum class TextureSwizzle : i32 {
        Zero,
        One,
        R,
        G,
        B,
        A,
    };

    struct TextureSwizzleMapping {
        TextureSwizzle r{TextureSwizzle::R};
        TextureSwizzle g{TextureSwizzle::G};
        TextureSwizzle b{TextureSwizzle::B};
        TextureSwizzle a{TextureSwizzle::A};
    };

    enum class TextureDepthStencilMode : i32 {
        Depth,
        Stencil,
    };

    struct PackagedTextureLayout {
        u8 layout[8];
    };

    class MemoryPool;
    class Texture;

    using StorageClass = i32;

    class TextureBuilder {
      private:
        friend Texture;

        Device *device;
        TextureFlags flags{}; //!< TODO VAL
        TextureTarget target{TextureTarget::Target2D}; //!< TODO VAL
        vk::Extent3D size{0, 1, 1}; //!< TODO VAL
        u32 levels{1};
        Format format{Format::None};
        i32 samples{1};
        TextureSwizzleMapping swizzleMapping{};
        TextureDepthStencilMode depthStencilMode{TextureDepthStencilMode::Depth}; //!< TODO VAL
        MemoryPool *memoryPool;
        i64 memoryOffset;
        const void *packagedTextureData{};
        PackagedTextureLayout packagedTextureLayout{};
        u64 stride;
        u32 glTextureName;

        std::pair<u32, u32> GetAxisSamples() const;

        u8 CalcTileHeight() const;

        u8 CalcTileDepth() const;

      public:
        TextureBuilder(ApiVersion version);

        void SetDevice(Device *pDevice);

        void SetFlags(TextureFlags pFlags);

        void SetTarget(TextureTarget pTarget);

        void SetWidth(i32 width);

        void SetHeight(i32 height);

        void SetDepth(i32 depth);

        void SetSize1D(i32 width);

        void SetSize2D(i32 width, i32 height);

        void SetSize3D(i32 width, i32 height, i32 depth);

        void SetLevels(i32 numLevels);

        void SetFormat(Format pFormat);

        void SetSamples(i32 pSamples);

        void SetSwizzle(TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a);

        void SetDepthStencilMode(TextureDepthStencilMode mode);

        u64 GetStorageSize() const;

        u64 GetStorageAlignment() const;

        void SetStorage(MemoryPool *pool, i64 offset);

        void SetPackagedTextureData(const void *data);

        void SetPackagedTextureLayout(const PackagedTextureLayout *layout);

        void SetStride(i64 pStride);

        void SetGLTextureName(u32 name);

        StorageClass GetStorageClass() const;

        const Device *GetDevice() const;

        TextureFlags GetFlags() const;

        TextureTarget GetTarget() const;

        i32 GetWidth() const;

        i32 GetHeight() const;

        i32 GetDepth() const;

        i32 GetLevels() const;

        Format GetFormat() const;

        i32 GetSamples() const;

        void GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const;

        TextureDepthStencilMode GetDepthStencilMode() const;

        const void *GetPackagedTextureData() const;

        bool GetPackagedTextureLayout(PackagedTextureLayout *layout) const;

        i64 GetStride() const;

//        void GetSparseTileLayout(TextureSparseTileLayout *tileLayout) const;

        u32 GetGLTextureName() const;

        u64 GetZCullStorageSize() const;

        const MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;
    };
    NNVK_VERSIONED_STRUCT(TextureBuilder, 0x80);

    class TextureView {
      private:
        i16 baseLevel{};
        i16 levels{1};
        i16 minLayer{};
        i16 layers{1};
        Format format{};
        TextureSwizzleMapping swizzleMapping{};
        TextureDepthStencilMode depthStencilMode{TextureDepthStencilMode::Depth};
        TextureTarget target{TextureTarget::Target2D}; // CHECK]

        struct WrittenMembers {
            bool level : 1;
            bool layer : 1;
            bool format : 1;
            bool swizzle : 1;
            bool depthStencilMode : 1;
            bool target : 1;

        } written{};

      public:
        TextureView(ApiVersion version);

        void SetLevels(i32 pBaseLevel, i32 numLevels);

        void SetLayers(i32 pMinLayer, i32 numLayers);

        void SetFormat(Format pFormat);

        void SetSwizzle(TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a);

        void SetDepthStencilMode(TextureDepthStencilMode mode);

        void SetTarget(TextureTarget pTarget);

        bool GetLevels(i32 *pBaseLevel, i32 *numLevels) const;

        bool GetLayers(i32 *pMinLayer, i32 *numLayers) const;

        bool GetFormat(Format *pFormat) const;

        bool GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const;

        bool GetDepthStencilMode(TextureDepthStencilMode *mode) const;

        bool GetTarget(TextureTarget *pTarget) const;
    };
    NNVK_VERSIONED_STRUCT(TextureView, 0x28);

    struct CopyRegion {
        i32 xOffset;
        i32 yOffset;
        i32 zOffset;
        i32 width;
        i32 height;
        i32 depth;
    };

    using TextureAddress = u64;

    class Texture {
      private:
        const char *debugLabel{};
        Device *device;
        TextureFlags flags;
        TextureTarget target;
        vk::Extent3D size;
        u32 levels;
        Format format;
        i32 samples;
        TextureSwizzleMapping swizzleMapping;
        TextureDepthStencilMode depthStencilMode;
        MemoryPool *memoryPool;
        i64 memoryOffset;
        u64 stride;
        u64 storageSize;
        StorageClass storageClass;

        VkCore &vkCore;

      public:
        Texture(ApiVersion version, VkCore &vkCore, const TextureBuilder &builder);

        ~Texture();

        size_t GetZCullStorageSize() const;

        void SetDebugLabel(const char *label);

        StorageClass GetStorageClass() const;

        i64 GetViewOffset(TextureView *view) const;

        TextureFlags GetFlags() const;

        TextureTarget GetTarget() const;

        i32 GetWidth() const;

        i32 GetHeight() const;

        i32 GetDepth() const;

        i32 GetLevels() const;

        Format GetFormat() const;

        i32 GetSamples() const;

        void GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const;

        TextureDepthStencilMode GetDepthStencilMode() const;

        i64 GetStride() const;

        TextureAddress GetTextureAddress() const;

        //void GetSparseTileLayout(TextureSparseTileLayout *tileLayout) const;
        void WriteTexels(const TextureView *view, const CopyRegion *region,
                         const void *p);

        void WriteTexelsStrided(const TextureView *view, const CopyRegion *region,
                                const void *p, i64 o1, i64 o2);

        void ReadTexels(const TextureView *view, const CopyRegion *region,
                        void *p);

        void ReadTexelsStrided(const TextureView *view, const CopyRegion *region,
                               void *p, i64 o1, i64 o2);

        void FlushTexels(const TextureView *view, const CopyRegion *region);

        void InvalidateTexels(const TextureView *view, const CopyRegion *region);

        MemoryPool *GetMemoryPool() const;

        i64 GetMemoryOffset() const;

        i32 GetStorageSize() const;

        bool Compare(const Texture *texture) const;

        u64 GetDebugID() const;
    };
    NNVK_VERSIONED_STRUCT(Texture, 0xC0);
}
