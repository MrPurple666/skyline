#include <new>
#include <stdexcept>
#include <memory>
#include <fmt/format.h>
#include "logging.h"
#include "nnvk.h"
#include "sampler.h"

namespace nnvk {
    SamplerBuilder::SamplerBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(SamplerBuilder);
    }

    void SamplerBuilder::SetDevice(const Device *pDevice) {
        device = pDevice;
    }

    void SamplerBuilder::SetMinMagFilter(MinFilter min, MagFilter mag) {
        minFilter = min;
        magFilter = mag;
    }

    void SamplerBuilder::SetWrapMode(WrapMode s, WrapMode t, WrapMode r) {
        wrapMode = {s, t, r};
    }

    void SamplerBuilder::SetLodClamp(f32 min, f32 max) {
        lodClampMin = min;
        lodClampMax = max;
    }

    void SamplerBuilder::SetLodBias(f32 bias) {
        lodBias = bias;
    }

    void SamplerBuilder::SetCompare(CompareMode mode, CompareFunc func) {
        compareMode = mode;
        compareFunc = func;
    }

    void SamplerBuilder::SetBorderColor(const f32 *pBorderColor) {
        f32BorderColor = true;
        std::copy(pBorderColor, pBorderColor + 4, this->borderColor.begin());
    }

    void SamplerBuilder::SetBorderColori(const i32 *pBorderColor) {
        f32BorderColor = false;
        std::copy(pBorderColor, pBorderColor + 4, this->borderColori.begin());
    }

    void SamplerBuilder::SetBorderColorui(const u32 *pBorderColor) {
        f32BorderColor = false;
        std::copy(pBorderColor, pBorderColor + 4, this->borderColorui.begin());
    }

    void SamplerBuilder::SetMaxAnisotropy(f32 maxAniso) {
        maxAnisotropy = maxAniso;
    }

    void SamplerBuilder::SetReductionFilter(SamplerReduction filter) {
        reductionFilter = filter;
    }

    void SamplerBuilder::SetLodSnap(f32 f) {
        lodSnap = f;
    }

    const Device *SamplerBuilder::GetDevice() const {
        return device;
    }

    void SamplerBuilder::GetMinMagFilter(MinFilter *min, MagFilter *mag) const {
        *min = minFilter;
        *mag = magFilter;
    }

    void SamplerBuilder::GetWrapMode(WrapMode *s, WrapMode *t, WrapMode *r) const {
        *s = wrapMode[0];
        *t = wrapMode[1];
        *r = wrapMode[2];
    }

    void SamplerBuilder::GetLodClamp(f32 *min, f32 *max) const {
        *min = lodClampMin;
        *max = lodClampMax;
    }

    f32 SamplerBuilder::GetLodBias() const {
        return lodBias;
    }

    void SamplerBuilder::GetCompare(CompareMode *mode, CompareFunc *func) const {
        *mode = compareMode;
        *func = compareFunc;
    }

    void SamplerBuilder::GetBorderColor(f32 *pBorderColor) const {
        std::copy(borderColor.begin(), borderColor.end(), pBorderColor);
    }

    void SamplerBuilder::GetBorderColori(i32 *pBorderColor) const {
        std::copy(borderColori.begin(), borderColori.end(), pBorderColor);
    }

    void SamplerBuilder::GetBorderColorui(u32 *pBorderColor) const {
        std::copy(borderColorui.begin(), borderColorui.end(), pBorderColor);
    }

    f32 SamplerBuilder::GetMaxAnisotropy() const {
        return maxAnisotropy;
    }

    SamplerReduction SamplerBuilder::GetReductionFilter() const {
        return reductionFilter;
    }

    f32 SamplerBuilder::GetLodSnap() const {
        return lodSnap;
    }

    /* Wrappers */
    void Context::SamplerBuilderSetDefaults(SamplerBuilder *builder) {
        new (builder) SamplerBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetDevice, const Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, SetMinMagFilter, MinFilter, MagFilter)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, SamplerBuilder, SetWrapMode, WrapMode, WrapMode, WrapMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, SetLodClamp, f32, f32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetLodBias, f32)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, SetCompare, CompareMode, CompareFunc)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetBorderColor, const f32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetBorderColori, const i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetBorderColorui, const u32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetMaxAnisotropy, f32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetReductionFilter, SamplerReduction)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, SetLodSnap, f32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, SamplerBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, GetMinMagFilter, MinFilter *, MagFilter *)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, SamplerBuilder, GetWrapMode, WrapMode *, WrapMode *, WrapMode *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, GetLodClamp, f32 *, f32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(f32, SamplerBuilder, GetLodBias)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, SamplerBuilder, GetCompare, CompareMode *, CompareFunc *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, GetBorderColor, f32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, GetBorderColori, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, SamplerBuilder, GetBorderColorui, u32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(f32, SamplerBuilder, GetMaxAnisotropy)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(SamplerReduction, SamplerBuilder, GetReductionFilter)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(f32, SamplerBuilder, GetLodSnap)
    /* End wrappers */

    static vk::Filter ConvertMagFilter(MagFilter filter) {
        switch (filter) {
            case MagFilter::Nearest:
                return vk::Filter::eNearest;
            case MagFilter::Linear:
                return vk::Filter::eLinear;
            default:
                throw std::runtime_error("Invalid mag filter");
        }
    }

    static std::pair<vk::Filter, vk::SamplerMipmapMode> ConvertMinFilter(MinFilter filter) {
        switch (filter) {
            case MinFilter::Nearest:
                return {vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest};
            case MinFilter::Linear:
                return {vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest};
            case MinFilter::NearestMipmapNearest:
                return {vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest};
            case MinFilter::LinearMipmapNearest:
                return {vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest};
            case MinFilter::NearestMipmapLinear:
                return {vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear};
            case MinFilter::LinearMipmapLinear:
                return {vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear};
            default:
                throw std::runtime_error("Invalid min filter");
        }
    }

    static bool NeedsForcedMipLodClamp(MinFilter filter) {
        switch (filter) {
            case MinFilter::Nearest:
            case MinFilter::Linear:
                return true;
            case MinFilter::NearestMipmapNearest:
            case MinFilter::LinearMipmapNearest:
            case MinFilter::NearestMipmapLinear:
            case MinFilter::LinearMipmapLinear:
                return false;
            default:
                throw std::runtime_error("Invalid min filter");
        }
    }

    static vk::SamplerAddressMode ConvertWrapMode(WrapMode mode) {
        switch (mode) {
            case WrapMode::Clamp:
                return vk::SamplerAddressMode::eClampToEdge; // TODO: Clamp
            case WrapMode::Repeat:
                return vk::SamplerAddressMode::eRepeat;
            case WrapMode::MirrorClamp:
                return vk::SamplerAddressMode::eClampToEdge; // TODO: MirrorClamp
            case WrapMode::MirrorClampToEdge:
                return vk::SamplerAddressMode::eMirrorClampToEdgeKHR;
            case WrapMode::MirrorClampToBorder:
                return vk::SamplerAddressMode::eClampToBorder; // TODO: MirrorClampToBorder
            case WrapMode::ClampToBorder:
                return vk::SamplerAddressMode::eClampToBorder;
            case WrapMode::MirroredRepeat:
                return vk::SamplerAddressMode::eMirroredRepeat;
            case WrapMode::ClampToEdge:
                return vk::SamplerAddressMode::eClampToEdge;
            default:
                throw std::runtime_error("Invalid wrap mode");
        }
    }

    static vk::CompareOp ConvertCompareFunc(CompareFunc func) {
        switch (func) {
            case CompareFunc::Never:
                return vk::CompareOp::eNever;
            case CompareFunc::Less:
                return vk::CompareOp::eLess;
            case CompareFunc::Equal:
                return vk::CompareOp::eEqual;
            case CompareFunc::LEqual:
                return vk::CompareOp::eLessOrEqual;
            case CompareFunc::Greater:
                return vk::CompareOp::eGreater;
            case CompareFunc::NotEqual:
                return vk::CompareOp::eNotEqual;
            case CompareFunc::GEqual:
                return vk::CompareOp::eGreaterOrEqual;
            case CompareFunc::Always:
                return vk::CompareOp::eAlways;
            default:
                throw std::runtime_error("Invalid compare func");
        }
    }

    static vk::BorderColor EstimateBorderColorFloat(std::array<f32, 4> color) {
        if (color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f && color[3] == 0.0f)
            return vk::BorderColor::eFloatTransparentBlack;
        else if (color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f && color[3] == 1.0f)
            return vk::BorderColor::eFloatOpaqueBlack;
        else if (color[0] == 1.0f && color[1] == 1.0f && color[2] == 1.0f && color[3] == 1.0f)
            return vk::BorderColor::eFloatOpaqueWhite;
        else
            return vk::BorderColor::eFloatTransparentBlack; //vk::BorderColor::eFloatCustomEXT;
    }

    static vk::BorderColor EstimateBorderColorInt(std::array<u32, 4> color) {
        throw std::runtime_error("Not implemented");
    }

    Sampler::Sampler(ApiVersion version, VkCore &vkCore, const SamplerBuilder &builder)
        : minFilter{builder.minFilter}, magFilter{builder.magFilter},
          wrapMode{builder.wrapMode},
          lodClampMin{builder.lodClampMin}, lodClampMax{builder.lodClampMax}, lodBias{builder.lodBias},
          compareMode{builder.compareMode}, compareFunc{builder.compareFunc},
          borderColor{builder.borderColor},
          maxAnisotropy{builder.maxAnisotropy},
          reductionFilter{builder.reductionFilter},
          sampler{std::make_unique<vk::raii::Sampler>(vkCore.device, vk::SamplerCreateInfo{
              .magFilter = ConvertMagFilter(builder.magFilter),
              .minFilter = ConvertMinFilter(builder.minFilter).first,
              .mipmapMode = ConvertMinFilter(builder.minFilter).second,
              .addressModeU = ConvertWrapMode(builder.wrapMode[0]),
              .addressModeV = ConvertWrapMode(builder.wrapMode[1]),
              .addressModeW = ConvertWrapMode(builder.wrapMode[2]),
              .mipLodBias = builder.lodBias,
              .anisotropyEnable = builder.maxAnisotropy > 1.0f,
              .maxAnisotropy = builder.maxAnisotropy,
              .compareEnable = builder.compareMode != CompareMode::None,
              .compareOp = ConvertCompareFunc(builder.compareFunc),
              .minLod = NeedsForcedMipLodClamp(builder.minFilter) ? 0.0f : builder.lodClampMin,
              .maxLod = NeedsForcedMipLodClamp(builder.minFilter) ? 0.25f : builder.lodClampMax,
              .borderColor = builder.f32BorderColor ? EstimateBorderColorFloat(builder.borderColor) :
                                                        EstimateBorderColorInt(builder.borderColorui),
              .unnormalizedCoordinates = false,
          })} {
        NNVK_FILL_VERSIONED_STRUCT(Sampler);
    }

    Sampler::~Sampler() {}

    void Sampler::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    void Sampler::GetMinMagFilter(MinFilter *min, MagFilter *mag) const {
        *min = minFilter;
        *mag = magFilter;
    }

    void Sampler::GetWrapMode(WrapMode *s, WrapMode *t, WrapMode *r) const {
        *s = wrapMode[0];
        *t = wrapMode[1];
        *r = wrapMode[2];
    }

    void Sampler::GetLodClamp(f32 *min, f32 *max) const {
        *min = lodClampMin;
        *max = lodClampMax;
    }

    f32 Sampler::GetLodBias() const {
        return lodBias;
    }

    void Sampler::GetCompare(CompareMode *mode, CompareFunc *func) const {
        *mode = compareMode;
        *func = compareFunc;
    }

    void Sampler::GetBorderColor(f32 *pBorderColor) const {
        std::copy(borderColor.begin(), borderColor.end(), pBorderColor);
    }

    void Sampler::GetBorderColori(i32 *pBorderColor) const {
        std::copy(borderColori.begin(), borderColori.end(), pBorderColor);
    }

    void Sampler::GetBorderColorui(u32 *pBorderColor) const {
        std::copy(borderColorui.begin(), borderColorui.end(), pBorderColor);
    }

    f32 Sampler::GetMaxAnisotropy() const {
        return maxAnisotropy;
    }

    SamplerReduction Sampler::GetReductionFilter() const {
        return reductionFilter;
    }

    bool Sampler::Compare(const Sampler *other) const {
        return minFilter == other->minFilter &&
               magFilter == other->magFilter &&
               wrapMode == other->wrapMode &&
               lodClampMin == other->lodClampMin &&
               lodClampMax == other->lodClampMax &&
               lodBias == other->lodBias &&
               compareMode == other->compareMode &&
               compareFunc == other->compareFunc &&
               borderColor == other->borderColor &&
               maxAnisotropy == other->maxAnisotropy &&
               reductionFilter == other->reductionFilter;
    }

    u64 Sampler::GetDebugID() const {
        throw std::runtime_error("Not implemented");
    }

    /* Wrappers */
    bool Context::SamplerInitialize(Sampler *pool, const SamplerBuilder *builder) {
        new (pool) Sampler(apiVersion, vkCore, *builder);
        return true;
    }

    void Context::SamplerFinalize(Sampler *pool) {
        pool->~Sampler();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Sampler, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Sampler, GetMinMagFilter, MinFilter *, MagFilter *)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Sampler, GetWrapMode, WrapMode *, WrapMode *, WrapMode *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Sampler, GetLodClamp, f32 *, f32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(f32, Sampler, GetLodBias)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, Sampler, GetCompare, CompareMode *, CompareFunc *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Sampler, GetBorderColor, f32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Sampler, GetBorderColori, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Sampler, GetBorderColorui, u32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(f32, Sampler, GetMaxAnisotropy)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(SamplerReduction, Sampler, GetReductionFilter)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, Sampler, Compare, const Sampler *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Sampler, GetDebugID)
    /* End wrappers */
}