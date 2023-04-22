// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <array>
#include <memory>
#include <vulkan/vulkan_raii.hpp>
#include "types.h"
#include "versioning.h"

namespace nnvk {
    struct VkCore;

    enum class MinFilter : i32 {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear,
    };

    enum class MagFilter : i32 {
        Nearest,
        Linear,
    };

    enum class WrapMode : i32 {
        Clamp,
        Repeat,
        MirrorClamp,
        MirrorClampToEdge,
        MirrorClampToBorder,
        ClampToBorder,
        MirroredRepeat,
        ClampToEdge,
    };

    enum class CompareMode : i32 {
        None,
        CompareRToTexture,
    };

    enum class CompareFunc : i32 {
        Never = 1,
        Less,
        Equal,
        LEqual,
        Greater,
        NotEqual,
        GEqual,
        Always,
    };

    enum class SamplerReduction : i32 {
        Average,
        Min,
        Max
    };

    class Device;
    class Sampler;

    class SamplerBuilder {
      private:
        friend Sampler;

        const Device *device{};
        MinFilter minFilter{MinFilter::Nearest};
        MagFilter magFilter{MagFilter::Nearest};
        std::array<WrapMode, 3> wrapMode{{WrapMode::Repeat, WrapMode::Repeat, WrapMode::Repeat}};
        f32 lodClampMin{0.0f};
        f32 lodClampMax{1000.0f};
        f32 lodBias{0.0f};
        CompareMode compareMode{CompareMode::None};
        CompareFunc compareFunc{CompareFunc::Less};
        union {
            std::array<f32, 4> borderColor{{0.0f, 0.0f, 0.0f, 0.0f}};
            std::array<i32, 4> borderColori;
            std::array<u32, 4> borderColorui;
        };
        bool f32BorderColor{true};
        f32 maxAnisotropy{1.0f};
        SamplerReduction reductionFilter{SamplerReduction::Average};
        f32 lodSnap{0.0f};

      public:
        SamplerBuilder(ApiVersion version);

        void SetDevice(const Device *pDevice);

        void SetMinMagFilter(MinFilter min, MagFilter mag);

        void SetWrapMode(WrapMode s, WrapMode t, WrapMode r);

        void SetLodClamp(f32 min, f32 max);

        void SetLodBias(f32 bias);

        void SetCompare(CompareMode mode, CompareFunc func);

        void SetBorderColor(const f32 *pBorderColor);

        void SetBorderColori(const i32 *pBorderColor);

        void SetBorderColorui(const u32 *pBorderColor);

        void SetMaxAnisotropy(f32 maxAniso);

        void SetReductionFilter(SamplerReduction filter);

        void SetLodSnap(f32 f);

        const Device *GetDevice() const;

        void GetMinMagFilter(MinFilter *min, MagFilter *mag) const;

        void GetWrapMode(WrapMode *s, WrapMode *t, WrapMode *r) const;

        void GetLodClamp(f32 *min, f32 *max) const;

        f32 GetLodBias() const;

        void GetCompare(CompareMode *mode, CompareFunc *func) const;

        void GetBorderColor(f32 *borderColor) const;

        void GetBorderColori(i32 *borderColor) const;

        void GetBorderColorui(u32 *borderColor) const;

        f32 GetMaxAnisotropy() const;

        SamplerReduction GetReductionFilter() const;

        f32 GetLodSnap() const;
    };
    NNVK_VERSIONED_STRUCT(SamplerBuilder, 0x60);

    class Sampler {
      private:
        const char *debugLabel{};
        std::array<WrapMode, 3> wrapMode;
        MinFilter minFilter;
        MagFilter magFilter;
        f32 lodClampMin;
        f32 lodClampMax;
        f32 lodBias;
        CompareMode compareMode;
        CompareFunc compareFunc;
        union {
            std::array<f32, 4> borderColor;
            std::array<i32, 4> borderColori;
            std::array<u32, 4> borderColorui;
        };
        f32 maxAnisotropy;
        SamplerReduction reductionFilter;

        // This may break SMO etc as they hash samplers?
        // Unique PTR is used as otherwise
        std::unique_ptr<vk::raii::Sampler> sampler{};

      public:
        Sampler(ApiVersion version, VkCore &vkCore, const SamplerBuilder &builder);

        ~Sampler();

        void SetDebugLabel(const char *label);

        void GetMinMagFilter(MinFilter *min, MagFilter *mag) const;

        void GetWrapMode(WrapMode *s, WrapMode *t, WrapMode *r) const;

        void GetLodClamp(f32 *min, f32 *max) const;

        f32 GetLodBias() const;

        void GetCompare(CompareMode *mode, CompareFunc *func) const;

        void GetBorderColor(f32 *pBorderColor) const;

        void GetBorderColori(i32 *pBorderColor) const;

        void GetBorderColorui(u32 *pBorderColor) const;

        f32 GetMaxAnisotropy() const;

        SamplerReduction GetReductionFilter() const;

        bool Compare(const Sampler *other) const;

        u64 GetDebugID() const;
    };
    NNVK_VERSIONED_STRUCT(Sampler, 0x60); // TODO: smaller in new nvn, bisect ver
}
