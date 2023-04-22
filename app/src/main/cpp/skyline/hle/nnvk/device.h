// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include "types.h"
#include "versioning.h"
#include "format.h"
#include "texture.h"

namespace nnvk {
    struct DeviceFlags {
        bool debugEnableLevel2 : 1;
        bool debugSkipCallsOnError : 1;
        bool debugEnableLevel3 : 1;
        bool deferredFinalize : 1;
        bool debugEnableLevel4 : 1;
        bool debugEnableLevel0 : 1;
        bool debugEnableLevel1 : 1;
        bool deferredFirmwareMemoryReclaim : 1;
        bool enableSeperateSamplerTextureSupport : 1;
        u32 _pad_ : 23;
    };
    static_assert(sizeof(DeviceFlags) == 4);

    class Device;

    class DeviceBuilder {
      private:
        friend Device;

        DeviceFlags flags{};

      public:
        DeviceBuilder(ApiVersion version);

        void SetFlags(DeviceFlags newFlags);

        DeviceFlags GetFlags() const;
    };
    NNVK_VERSIONED_STRUCT(DeviceBuilder, 0x40);

    enum class DeviceInfo : i32 {
        ApiMajorVersion,
        ApiMinorVersion,
        UniformBufferBindingsPerStage,
        MaxUniformBufferSize,
        UniformBufferAlignment,
        ColorBufferBindings,
        VertexBufferBindings,
        TransformFeedbackBufferBindings,
        ShaderStorageBufferBindingsPerStage,
        TextureBindingsPerStage,
        CounterAlignment,
        TransformFeedbackBufferAlignment,
        TransformFeedbackControlAlignment,
        IndirectDrawAlignment,
        VertexAttributes,
        TextureDescriptorSize,
        SamplerDescriptorSize,
        ReservedTextureDescriptors,
        ReservedSamplerDescriptors,
        CommandBufferCommandAlignment,
        CommandBufferControlAlignment,
        CommandBufferMinCommandSize,
        CommandBufferMinControlSize,
        ShaderScratchMemoryScaleFactorMinimum,
        ShaderScratchMemoryScaleFactorRecommended,
        ShaderScratchMemoryAlignment,
        ShaderScratchMemoryGranularity,
        MaxTextureAnisotropy,
        MaxComputeWorkGroupSizeX,
        MaxComputeWorkGroupSizeY,
        MaxComputeWorkGroupSizeZ,
        MaxComputeWorkGroupSizeThreads,
        MaxComputeDispatchWorkGroupsX,
        MaxComputeDispatchWorkGroupsY,
        MaxComputeDispatchWorkGroupsZ,
        ImageBindingsPerStage,
        MaxTexturePoolSize,
        MaxSamplerPoolSize,
        MaxViewports,
        MempoolTextureObjectPageAlignment,
        SupportsMinMaxFiltering,
        SupportsStencil8Format,
        SupportsAstcFormats,
        L2Size,
        MaxTextureLevels,
        MaxTextureLayers,
        GlslcMaxSupportedGpuCodeMajorVersion,
        GlslcMinSupportedGpuCodeMajorVersion,
        GlslcMaxSupportedGpuCodeMinorVersion,
        GlslcMinSupportedGpuCodeMinorVersion,
        SupportsConservativeRaster,
        SubpixelBits,
        MaxSubpixelBiasBits,
        IndirectDispatchAlignment,
        ZcullSaveRestoreAlignment,
        ShaderScratchMemoryComputeScaleFactorMinimum,
        LinearTextureStrideAlignment,
        LinearRenderTargetStrideAlignment,
        MemoryPoolPageSize,
        SupportsZeroFromUnmappedVirtualPoolPages,
        UniformBufferUpdateAlignment,
        MaxTextureSize,
        MaxBufferTextureSize,
        Max3DTextureSize,
        MaxCubeMapTextureSize,
        MaxRectangleTextureSize,
        SupportsPassthroughGeometryShaders,
        SupportsViewportSwizzle,
        SupportsSparseTiledPackagedTextures,
        SupportsAdvancedBlendModes,
        MaxPresentInterval,
        SupportsDrawTexture,
        SupportsTargetIndependentRasterization,
        SupportsFragmentCoverageToColor,
        SupportsPostDepthCoverage,
        SupportsImagesUsingTextureHandles,
        SupportsSampleLocations,
        MaxSampleLocationTableEntries,
        ShaderCodeMemoryPoolPaddingSize,
        MaxPatchSize,
        QueueCommandMemoryGranularity,
        QueueCommandMemoryMinSize,
        QueueCommandMemoryDefaultSize,
        QueueComputeMemoryGranularity,
        QueueComputeMemoryMinSize,
        QueueComputeMemoryDefaultSize,
        QueueCommandMemoryMinFlushThreshold,
        SupportsFragmentShaderInterlock,
        MaxTexturesPerWindow,
        MinTexturesPerWindow,
        SupportsDebugLayer,
        QueueControlMemoryMinSize,
        QueueControlMemoryDefaultSize,
        QueueControlMemoryGranularity,
        SeparateTextureBindingsPerStage,
        SeparateSamplerBindingsPerStage,
        DebugGroupsMaxDomainId,
        EventsSupportReductionOperations,
        // Present in NVN 55.12
        Unk0,
        Unk1,
        Unk2,
        Unk3,
        Unk4,
        Unk5,
        Unk6,
    };

    using TextureHandle = u64;
    using ImageHandle = u64;
    using SeparateSamplerHandle = u64;
    using SeparateTextureHandle = u64;

    using DebugDomainId = i32;
    using CommandHandle = u64;

    enum class WindowOriginMode : i32 {
        LowerLeft,
        UpperLeft
    };

    enum class DepthMode : i32 {
        NearIsMinusW,
        NearIsZero
    };

    struct CounterData {
        u64 counter;
        u64 timestamp;
    };

    enum class DebugObjectType : i32 {
        Window,
        Queue,
        CommandBuffer,
        MemoryPool,
        Sync = 7,
        Program = 9,
        TexturePool,
        SamplerPool,
    };

    class Device {
      private:
        const char *debugLabel{};
        WindowOriginMode windowOriginMode{};
        DepthMode depthMode{};

        bool enableDeferredFinalize{};
        bool enableDeferredFirmwareMemoryReclaim{};
        bool enableSeperateSamplerTextureSupport{};

      public:
        texture::VirtualTextureManager textureManager;

        Device(ApiVersion version, const DeviceBuilder &builder, VkCore &vkCore);

        ~Device();

        void SetDebugLabel(const char *label);

        void GetInteger(ApiVersion version, DeviceInfo pname, i32 *v) const;

        u64 GetCurrentTimestampInNanoseconds() const;

        void SetIntermediateShaderCache(i32 i);

        TextureHandle GetTextureHandle(i32 textureId, i32 samplerId) const;

        TextureHandle GetTexelFetchHandle(i32 textureId) const;

        ImageHandle GetImageHandle(i32 textureId) const;

        void InstallDebugCallback(const void *callback, void *callbackData, bool enable);

        DebugDomainId GenerateDebugDomainId(const char *s) const;

        void SetWindowOriginMode(WindowOriginMode mode);

        void SetDepthMode(DepthMode mode);

        bool RegisterFastClearColor(const float *color, Format format);

        bool RegisterFastClearColori(const i32 *color, Format format);

        bool RegisterFastClearColorui(const u32 *color, Format format);

        bool RegisterFastClearDepth(f32 f);

        WindowOriginMode GetWindowOriginMode() const;

        DepthMode GetDepthMode() const;

        u64 GetTimestampInNanoseconds(const CounterData *counterData) const;

        void ApplyDeferredFinalizes(i32 i);

        void FinalizeCommandHandle(CommandHandle handles);

        void WalkDebugDatabase(DebugObjectType type, const void *callback, void *callbackData) const;

        SeparateTextureHandle GetSeparateTextureHandle(i32 textureId) const;

        SeparateSamplerHandle GetSeparateSamplerHandle(i32 samplerId) const;

        bool IsExternalDebuggerAttached() const;
    };
    NNVK_VERSIONED_STRUCT(Device, 0x3000);
}
