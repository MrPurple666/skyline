#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "nnvk.h"
#include "logging.h"
#include "memory_pool.h"
#include "layout.h"
#include "utils.h"
#include "texture.h"

namespace nnvk {
    vk::Extent2D TextureBuilder::GetAxisSamples() const {
        if (target != TextureTarget::Target2DMultisample && target != TextureTarget::Target2DMultisampleArray)
            return {1, 1};

        switch (samples) {
            case 1:
                return {1, 1};
            case 2:
                return {1, 1};
            case 4:
                return {2, 2};
            case 8:
                return {4, 2};
            case 16:
                return {4, 4};
            default:
                throw std::runtime_error(fmt::format("Invalid sample count: {}", samples));
        }
    }

    std::pair<vk::Extent3D, bool> TextureBuilder::GetStorageDimensions() const {
        auto axisSamples{GetAxisSamples()};

        switch (target) {
            case TextureTarget::Target1D:
                return {{size.width, 1, 1}, false};
            case TextureTarget::Target2D:
                return {{size.width, size.height, 1}, false};
            case TextureTarget::Target3D:
                return {{size.width, size.height, size.depth}, false};
            case TextureTarget::Target1DArray:
                Logger::Error("HAD 1D");
                return {{size.width, 1, size.width}, true};
            case TextureTarget::Target2DArray:
                return {{size.width, size.height, size.depth}, true};
            case TextureTarget::Target2DMultisample:
                return {{size.width * axisSamples.width, size.height * axisSamples.height, 1}, false};
            case TextureTarget::Target2DMultisampleArray:
                return {{size.width * axisSamples.width, size.height * axisSamples.height, size.depth}, true};
            case TextureTarget::TargetRectangle:
                return {{size.width, size.height, 1}, false};
                break;
            case TextureTarget::TargetCubemap:
                return {{size.width, size.height, 6}, true};
            case TextureTarget::TargetCubemapArray:
                return {{size.width, size.height, size.depth * 6}, true};
            case TextureTarget::TargetBuffer:
                return {{size.width, 1, 1}, false};
        }
    }

    u8 TextureBuilder::CalcTileHeight() const {
        if (flags.videoDecode)
            return 2;

        if (flags.linear || target == TextureTarget::Target1D || target == TextureTarget::Target1DArray || target == TextureTarget::TargetBuffer)
            return 1;

        auto axisSamples{GetAxisSamples()};
        const auto &formatInfo{format::GetFormatInfo(format)};
        return layout::SelectBlockLinearTileHeight(size.height * axisSamples.height, formatInfo.blockHeight);
    }

    u8 TextureBuilder::CalcTileDepth() const {
        if (target != TextureTarget::Target3D)
            return 1;

        return layout::SelectBlockLinearTileDepth(size.depth);
    }

    TextureBuilder::TextureBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(TextureBuilder);
    }

    void TextureBuilder::SetDevice(Device *pDevice) {
        device = pDevice;
    }
    
    void TextureBuilder::SetFlags(TextureFlags pFlags) {
        flags = pFlags;
    }
    
    void TextureBuilder::SetTarget(TextureTarget pTarget) {
        target = pTarget;
    }
    
    void TextureBuilder::SetWidth(i32 width) {
        if (width < 0)
            throw std::length_error("width < 0");

        size.width = static_cast<u32>(width);
    }
    
    void TextureBuilder::SetHeight(i32 height) {
        if (height < 0)
            throw std::length_error("height < 0");

        size.height = static_cast<u32>(height);
    }
    
    void TextureBuilder::SetDepth(i32 depth) {
        if (depth < 0)
            throw std::length_error("depth < 0");

        size.depth = static_cast<u32>(depth);
    }
    
    void TextureBuilder::SetSize1D(i32 width) {
        if (width < 0)
            throw std::length_error("width < 0");

        size.width = static_cast<u32>(width);
    }
    
    void TextureBuilder::SetSize2D(i32 width, i32 height) {
        if (width < 0)
            throw std::length_error("width < 0");

        if (height < 0)
            throw std::length_error("height < 0");

        size.width = static_cast<u32>(width);
        size.height = static_cast<u32>(height);
    }
    
    void TextureBuilder::SetSize3D(i32 width, i32 height, i32 depth) {
        if (width < 0)
            throw std::length_error("width < 0");

        if (height < 0)
            throw std::length_error("height < 0");

        if (depth < 0)
            throw std::length_error("depth < 0");

        size.width = static_cast<u32>(width);
        size.height = static_cast<u32>(height);
        size.depth = static_cast<u32>(depth);
    }
    
    void TextureBuilder::SetLevels(i32 numLevels) {
        if (numLevels < 0)
            throw std::length_error("numLevels < 0");

        levels = static_cast<u32>(numLevels);
    }
    
    void TextureBuilder::SetFormat(Format pFormat) {
        format = pFormat;
    }
    
    void TextureBuilder::SetSamples(i32 pSamples) {
        samples = pSamples;
    }
    
    void TextureBuilder::SetSwizzle(TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
        swizzleMapping.r = r;
        swizzleMapping.g = g;
        swizzleMapping.b = b;
        swizzleMapping.a = a;
    }
    
    void TextureBuilder::SetDepthStencilMode(TextureDepthStencilMode mode) {
        depthStencilMode = mode;
    }
    
    u64 TextureBuilder::GetStorageSize() const {
        const auto &formatInfo{format::GetFormatInfo(format)};
        if (!formatInfo.texture)
            return 0;

        if (target == TextureTarget::TargetBuffer)
            return size.width * formatInfo.bytesPerBlock;

        if (flags.linear) {
            if (stride == 0)
                throw std::runtime_error("stride == 0");

            return stride * size.height;
        }


        auto [storageDimensions, hasLayers]{GetStorageDimensions()};
        u64 layerSize{layout::GetBlockLinearLayerSize({storageDimensions.width, storageDimensions.height, hasLayers ? 1 : storageDimensions.depth},
                                                      formatInfo.blockHeight, formatInfo.blockWidth, formatInfo.bytesPerBlock,
                                                      CalcTileHeight(), CalcTileDepth(), levels, hasLayers)};

        return layerSize * (hasLayers ? storageDimensions.depth : 1);
    }
    
    u64 TextureBuilder::GetStorageAlignment() const {
        if (target == TextureTarget::TargetBuffer) {
            u8 formatBpb{format::GetFormatInfo(format).bytesPerBlock};

            // Reduced alignment for 12BPP formats
            return formatBpb == 12 ? 4 : formatBpb;
        }


        if (flags.linearRenderTarget)
            return 0x80;
        else if (flags.linear)
            return 0x20;

        // TODO: impl big page size logic, for now just assume all use small pages
        return 0x200;
    }
    
    void TextureBuilder::SetStorage(MemoryPool *pool, i64 offset) {
        memoryPool = pool;
        memoryOffset = offset;
    }
    
    void TextureBuilder::SetPackagedTextureData(const void *data) {
        packagedTextureData = data;
    }
    
    void TextureBuilder::SetPackagedTextureLayout(const PackagedTextureLayout *layout) {
        throw std::runtime_error("TextureBuilder::SetPackagedTextureLayout() is not implemented");
        packagedTextureLayout = *layout;
    }
    
    void TextureBuilder::SetStride(i64 pStride) {
        if (stride < 0)
            throw std::length_error("stride < 0");
        stride = static_cast<u64>(pStride);
    }
    
    void TextureBuilder::SetGLTextureName(u32 name) {
        glTextureName = name;
    }
    
    StorageClass TextureBuilder::GetStorageClass() const {
        Logger::Error("not implemented");
        return 0;
    }

    const Device *TextureBuilder::GetDevice() const {
        return device;
    }
    
    TextureFlags TextureBuilder::GetFlags() const {
        return flags;
    }
    
    TextureTarget TextureBuilder::GetTarget() const {
        return target;
    }
    
    i32 TextureBuilder::GetWidth() const {
        return static_cast<i32>(size.width);
    }
    
    i32 TextureBuilder::GetHeight() const {
        return static_cast<i32>(size.height);
    }
    
    i32 TextureBuilder::GetDepth() const {
        return static_cast<i32>(size.depth);
    }
    
    i32 TextureBuilder::GetLevels() const {
        return static_cast<i32>(levels);
    }
    
    Format TextureBuilder::GetFormat() const {
        return format;
    }
    
    i32 TextureBuilder::GetSamples() const {
        return samples;
    }
    
    void TextureBuilder::GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const {
        *r = swizzleMapping.r;
        *g = swizzleMapping.g;
        *b = swizzleMapping.b;
        *a = swizzleMapping.a;
    }
    
    TextureDepthStencilMode TextureBuilder::GetDepthStencilMode() const {
        return depthStencilMode;
    }

    const void *TextureBuilder::GetPackagedTextureData() const {
        return packagedTextureData;
    }

    bool TextureBuilder::GetPackagedTextureLayout(PackagedTextureLayout *layout) const {
        throw std::runtime_error("TextureBuilder::GetPackagedTextureLayout() is not implemented");
        return false;
    }

    i64 TextureBuilder::GetStride() const {
        return static_cast<i64>(stride);
    }

    u32 TextureBuilder::GetGLTextureName() const {
        return glTextureName;
    }

    u64 TextureBuilder::GetZCullStorageSize() const {
        throw std::runtime_error("TextureBuilder::GetZCullStorageSize() is not implemented");
    }

    const MemoryPool *TextureBuilder::GetMemoryPool() const {
        return memoryPool;
    }

    i64 TextureBuilder::GetMemoryOffset() const {
        return memoryOffset;
    }
    
    /* Wrappers */
    void Context::TextureBuilderSetDefaults(TextureBuilder *builder) {
        new (builder) TextureBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetDevice, Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetFlags, TextureFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetTarget, TextureTarget)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetWidth, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetHeight, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetDepth, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetSize1D, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, TextureBuilder, SetSize2D, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, TextureBuilder, SetSize3D, i32, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetLevels, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetFormat, Format)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetSamples, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(void, TextureBuilder, SetSwizzle, TextureSwizzle, TextureSwizzle, TextureSwizzle, TextureSwizzle)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetDepthStencilMode, TextureDepthStencilMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, TextureBuilder, GetStorageSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, TextureBuilder, GetStorageAlignment)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, TextureBuilder, SetStorage, MemoryPool *, i64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetPackagedTextureData, const void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetPackagedTextureLayout, const PackagedTextureLayout *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetStride, i64)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureBuilder, SetGLTextureName, u32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(StorageClass, TextureBuilder, GetStorageClass)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, TextureBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureFlags, TextureBuilder, GetFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureTarget, TextureBuilder, GetTarget)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TextureBuilder, GetWidth)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TextureBuilder, GetHeight)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TextureBuilder, GetDepth)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TextureBuilder, GetLevels)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(Format, TextureBuilder, GetFormat)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, TextureBuilder, GetSamples)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(void, TextureBuilder, GetSwizzle, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureDepthStencilMode, TextureBuilder, GetDepthStencilMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const void *, TextureBuilder, GetPackagedTextureData)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, TextureBuilder, GetPackagedTextureLayout, PackagedTextureLayout *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, TextureBuilder, GetStride)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u32, TextureBuilder, GetGLTextureName)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, TextureBuilder, GetZCullStorageSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const MemoryPool *, TextureBuilder, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, TextureBuilder, GetMemoryOffset)
    /* End wrappers */


    TextureView::TextureView(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(TextureView);
    }

    void TextureView::SetLevels(i32 pBaseLevel, i32 numLevels) {
        baseLevel = static_cast<i16>(pBaseLevel);
        levels = static_cast<i16>(numLevels);
        written.level = true;
    }

    void TextureView::SetLayers(i32 pMinLayer, i32 numLayers) {
        minLayer = static_cast<i16>(pMinLayer);
        layers = static_cast<i16>(numLayers);
        written.layer = true;
    }

    void TextureView::SetFormat(Format pFormat) {
        format = pFormat;
        written.format = true;
    }

    void TextureView::SetSwizzle(TextureSwizzle r, TextureSwizzle g, TextureSwizzle b, TextureSwizzle a) {
        swizzleMapping = {r, g, b, a};
        written.swizzle = true;
    }

    void TextureView::SetDepthStencilMode(TextureDepthStencilMode mode) {
        depthStencilMode = mode;
        written.depthStencilMode = true;
    }

    void TextureView::SetTarget(TextureTarget pTarget) {
        target = pTarget;
        written.target = true;
    }

    bool TextureView::GetLevels(i32 *pBaseLevel, i32 *numLevels) const {
        *pBaseLevel = baseLevel;
        *numLevels = levels;
        return written.level;
    }

    bool TextureView::GetLayers(i32 *pMinLayer, i32 *numLayers) const {
        *pMinLayer = minLayer;
        *numLayers = layers;
        return written.layer;
    }

    bool TextureView::GetFormat(Format *pFormat) const {
        *pFormat = format;
        return written.format;
    }

    bool TextureView::GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const {
        *r = swizzleMapping.r;
        *g = swizzleMapping.g;
        *b = swizzleMapping.b;
        *a = swizzleMapping.a;
        return written.swizzle;
    }

    bool TextureView::GetDepthStencilMode(TextureDepthStencilMode *mode) const {
        *mode = depthStencilMode;
        return written.depthStencilMode;
    }

    bool TextureView::GetTarget(TextureTarget *pTarget) const {
        *pTarget = target;
        return written.target;
    }

    /* Wrappers */
    void Context::TextureViewSetDefaults(TextureView *view) {
        new (view) TextureView(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, TextureView, SetLevels, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, TextureView, SetLayers, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureView, SetFormat, Format)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(void, TextureView, SetSwizzle, TextureSwizzle, TextureSwizzle, TextureSwizzle, TextureSwizzle)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureView, SetDepthStencilMode, TextureDepthStencilMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, TextureView, SetTarget, TextureTarget)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, TextureView, GetLevels, i32 *, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, TextureView, GetLayers, i32 *, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, TextureView, GetFormat, Format *)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(bool, TextureView, GetSwizzle, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, TextureView, GetDepthStencilMode, TextureDepthStencilMode *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, TextureView, GetTarget, TextureTarget *)
    /* End wrappers */


    Texture::Texture(ApiVersion version, const TextureBuilder &builder)
        : device{builder.device},
          virtualTexture{device->textureManager.FindOrCreate(builder)},
          flags{builder.flags},
          target{builder.target},
          size{builder.size},
          format{builder.format},
          swizzleMapping{builder.swizzleMapping},
          depthStencilMode{builder.depthStencilMode},
          memoryPool{builder.memoryPool},
          memoryOffset{builder.memoryOffset} {
        NNVK_FILL_VERSIONED_STRUCT(Texture);
        // TODO HANDLE PACKAGED
    }

    Texture::~Texture() {}

    size_t Texture::GetZCullStorageSize() const {
        throw std::runtime_error("Texture::GetZCullStorageSize() is not implemented");
    }

    void Texture::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    StorageClass Texture::GetStorageClass() const {
        return virtualTexture->storageClass;
    }

    i64 Texture::GetViewOffset(TextureView *view) const {
        throw std::runtime_error("Texture::GetViewOffset() is not implemented");
    }

    TextureFlags Texture::GetFlags() const {
        return flags;
    }

    TextureTarget Texture::GetTarget() const {
        return target;
    }

    i32 Texture::GetWidth() const {
        return static_cast<i32>(size.width);
    }

    i32 Texture::GetHeight() const {
        return static_cast<i32>(size.height);
    }

    i32 Texture::GetDepth() const {
        return static_cast<i32>(size.depth);
    }

    i32 Texture::GetLevels() const {
        return static_cast<i32>(virtualTexture->levels);
    }

    Format Texture::GetFormat() const {
        return format;
    }

    i32 Texture::GetSamples() const {
        return virtualTexture->samples;
    }

    void Texture::GetSwizzle(TextureSwizzle *r, TextureSwizzle *g, TextureSwizzle *b, TextureSwizzle *a) const {
        *r = swizzleMapping.r;
        *g = swizzleMapping.g;
        *b = swizzleMapping.b;
        *a = swizzleMapping.a;
    }

    TextureDepthStencilMode Texture::GetDepthStencilMode() const {
        return depthStencilMode;
    }

    i64 Texture::GetStride() const {
        return static_cast<i64>(virtualTexture->stride);
    }

    TextureAddress Texture::GetTextureAddress() const {
        throw std::runtime_error("Texture::GetTextureAddress() is not implemented");
    }

    //void GetSparseTileLayout(TextureSparseTileLayout *tileLayout) const;
    void Texture::WriteTexels(const TextureView *view, const CopyRegion *region,
                     const void *p) {
        Logger::Error("Texture::WriteTexels() is not implemented");
    }

    void Texture::WriteTexelsStrided(const TextureView *view, const CopyRegion *region,
                                     const void *p, i64 o1, i64 o2) {
        Logger::Error("Texture::WriteTexelsStrided() is not implemented");
    }

    void Texture::ReadTexels(const TextureView *view, const CopyRegion *region,
                             void *p) {
        Logger::Error("Texture::ReadTexels() is not implemented");
    }

    void Texture::ReadTexelsStrided(const TextureView *view, const CopyRegion *region,
                                    void *p, i64 o1, i64 o2) {
        Logger::Error("Texture::ReadTexelsStrided() is not implemented");
    }

    void Texture::FlushTexels(const TextureView *view, const CopyRegion *region) {
        Logger::Error("Texture::FlushTexels() is not implemented");
    }

    void Texture::InvalidateTexels(const TextureView *view, const CopyRegion *region) {
        Logger::Error("Texture::InvalidateTexels() is not implemented");
    }

    MemoryPool *Texture::GetMemoryPool() const {
        return memoryPool;
    }

    i64 Texture::GetMemoryOffset() const {
        return memoryOffset;
    }

    i32 Texture::GetStorageSize() const {
        return static_cast<i32>(virtualTexture->storageSize);
    }

    bool Texture::Compare(const Texture *texture) const {
        // TODO: may not check storages
        return virtualTexture == texture->virtualTexture;
    }

    u64 Texture::GetDebugID() const {
        throw std::runtime_error("Texture::GetDebugID() is not implemented");
    }

    /* Wrappers */
    bool Context::TextureInitialize(Texture *texture, const TextureBuilder *builder) {
        new (texture) Texture(apiVersion, *builder);
        return true;
    }

    void Context::TextureFinalize(Texture *texture) {
        texture->~Texture();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_0(size_t, Texture, GetZCullStorageSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Texture, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(StorageClass, Texture, GetStorageClass)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(i64, Texture, GetViewOffset, TextureView *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureFlags, Texture, GetFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureTarget, Texture, GetTarget)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetWidth)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetHeight)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetDepth)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetLevels)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(Format, Texture, GetFormat)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetSamples)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(void, Texture, GetSwizzle, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *, TextureSwizzle *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureDepthStencilMode, Texture, GetDepthStencilMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, Texture, GetStride)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(TextureAddress, Texture, GetTextureAddress)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Texture, WriteTexels, const TextureView *, const CopyRegion *, const void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_5(void, Texture, WriteTexelsStrided, const TextureView *, const CopyRegion *, const void *, i64, i64)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Texture, ReadTexels, const TextureView *, const CopyRegion *, void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_5(void, Texture, ReadTexelsStrided, const TextureView *, const CopyRegion *, void *, i64, i64)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Texture, FlushTexels, const TextureView *, const CopyRegion *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Texture, InvalidateTexels, const TextureView *, const CopyRegion *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(MemoryPool *, Texture, GetMemoryPool)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i64, Texture, GetMemoryOffset)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Texture, GetStorageSize)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, Texture, Compare, const Texture *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Texture, GetDebugID)

    namespace texture {
        static vk::ImageType ConvertImageType(TextureTarget target) {
            switch (target) {
                case TextureTarget::Target1D:
                case TextureTarget::Target1DArray:
                    return vk::ImageType::e1D;
                case TextureTarget::Target2D:
                case TextureTarget::Target2DArray:
                case TextureTarget::Target2DMultisample:
                case TextureTarget::Target2DMultisampleArray:
                case TextureTarget::TargetCubemap:
                case TextureTarget::TargetCubemapArray:
                case TextureTarget::TargetRectangle:
                    return vk::ImageType::e2D;
                case TextureTarget::Target3D:
                    return vk::ImageType::e3D;
                default:
                    throw std::runtime_error("Invalid texture target");
            }
        }
        VirtualTexture::ImageAlias::AliasInfo::AliasInfo(Format format, vk::Extent2D size, TextureTarget target)
            : format{format}, size{size}, type{ConvertImageType(target)} {}

        static vk::SampleCountFlagBits ConvertSampleCount(i32 samples) {
            switch (samples) {
                case 1:
                    return vk::SampleCountFlagBits::e1;
                case 2:
                    return vk::SampleCountFlagBits::e2;
                case 4:
                    return vk::SampleCountFlagBits::e4;
                case 8:
                    return vk::SampleCountFlagBits::e8;
                case 16:
                    return vk::SampleCountFlagBits::e16;
                default:
                    throw std::runtime_error("Invalid sample count");
            }
        }

        vk::ImageCreateInfo VirtualTexture::GetImageAliasCreateInfo(const ImageAlias::AliasInfo &info) {
            vk::ImageCreateFlags createFlags{};

            if (formatList.size() > 1)
                createFlags |= vk::ImageCreateFlagBits::eMutableFormat;

            if (hasLayers)
                createFlags |= vk::ImageCreateFlagBits::e2DArrayCompatible;

            if (hasLayers && size.depth > 6)
                createFlags |= vk::ImageCreateFlagBits::eCubeCompatible;


            //  TODO handle block texel view
            const auto &formatInfo{format::GetFormatInfo(info.format)};

            vk::ImageUsageFlags createUsageFlags{vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
            if (usedAsStorage)
                createUsageFlags |= vk::ImageUsageFlagBits::eStorage;

            if (!formatInfo.IsCompressed()) {
                if (formatInfo.IsDepthStencil())
                    createUsageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
                else
                    createUsageFlags |= vk::ImageUsageFlagBits::eColorAttachment;
            }

            return {
                .flags = createFlags,
                .imageType = info.type,
                .format = formatInfo.vkFormat,
                .extent = vk::Extent3D{info.size.width, info.size.height, hasLayers ? 1 : size.depth},
                .mipLevels = levels,
                .arrayLayers = hasLayers ? size.depth : 1,
                .samples = ConvertSampleCount(samples),
                .tiling = flags.IsLinearLayout() ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal,
                .usage = createUsageFlags,
                .sharingMode = vk::SharingMode::eExclusive,
                .queueFamilyIndexCount = 0,
                .initialLayout = flags.IsLinearLayout() ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined
            };
        }

        void VirtualTexture::ReallocateMemory() {
            if (flags.IsLinearLayout() || target == TextureTarget::TargetBuffer) {
                vkAllocation.reset();
            } else {
                auto createInfo{GetImageAliasCreateInfo({format, {size.width, size.height}, target})};
                vkAllocation.emplace(vkCore.memoryManager.AllocateImageMemory(createInfo));
            }
        }

        VirtualTexture::VirtualTexture(VkCore &vkCore, const TextureBuilder &builder, TextureAddress address)
            : vkCore{vkCore},
              address{address},
              flags{builder.flags},
              target{builder.target},
              size{builder.size},
              levels{builder.levels},
              format{builder.format},
              samples{target == TextureTarget::Target2DMultisample || target == TextureTarget::Target2DMultisampleArray ? builder.samples : 1},
              stride{builder.stride},
              storageSize{builder.GetStorageSize()},
              storageClass{builder.GetStorageClass()},
              storageDimensions{builder.GetStorageDimensions().first},
              hasLayers{builder.GetStorageDimensions().second}, // STRUCT??
              tileHeight{builder.CalcTileHeight()},
              tileDepth{builder.CalcTileDepth()},
              memoryPool{builder.memoryPool},
              memoryOffset{static_cast<u64>(builder.memoryOffset)},
              memoryBuffer{vkCore.memoryManager.CreateAliasingBuffer(*memoryPool->buffer.value().vkMemory, memoryOffset, storageSize)} {
            ReallocateMemory();
            Logger::Error("Impl {}", static_cast<u32>(format));
            // UPLOAD DATA PACKAGED
        }

        bool VirtualTexture::IsCompatible(const TextureBuilder &builder) const {
            // TODO: LOGIC
            return true;
        }

        VirtualTextureManager::VirtualTextureManager(VkCore &vkCore) : vkCore{vkCore} {}

        // LOCK
        VirtualTextureHandle VirtualTextureManager::FindOrCreate(const TextureBuilder &builder) {
            TextureAddress address{builder.memoryPool->GetBufferAddress() + static_cast<u64>(builder.memoryOffset)};
            auto &matchedTextures{textures[address]};
            for (auto it{matchedTextures.begin()}; it != matchedTextures.end(); it++) {
                if (it->IsCompatible(builder)) {
                    Logger::Error("MATCH! need code!");
                    it->refs++;
                    return it;
                }
            }


            // Refcount is initialized to 1
            matchedTextures.emplace_back(vkCore, builder, address);
            return std::prev(matchedTextures.end());
        }

        void VirtualTextureManager::PutTexture(VirtualTextureHandle texture) {
            if (--texture->refs == 0) {
                auto &matchedTextures{textures[texture->address]};
                matchedTextures.erase(texture);
            }
        }
    }
}