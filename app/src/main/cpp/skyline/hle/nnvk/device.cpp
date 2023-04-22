#include <array>
#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "logging.h"
#include "nnvk.h"
#include "format.h"
#include "device.h"

namespace nnvk {
    DeviceBuilder::DeviceBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(DeviceBuilder);
    }

    void DeviceBuilder::SetFlags(DeviceFlags newFlags) {
        flags = newFlags;
    }

    DeviceFlags DeviceBuilder::GetFlags() const {
        return flags;
    }

    /* Wrappers */
    void Context::DeviceBuilderSetDefaults(DeviceBuilder *builder) {
        new (builder) DeviceBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, DeviceBuilder, SetFlags, DeviceFlags)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(DeviceFlags, DeviceBuilder, GetFlags)
    /* End wrappers */

    Device::Device(ApiVersion version, const DeviceBuilder &builder, VkCore &vkCore)
        : enableDeferredFinalize{builder.flags.deferredFinalize},
          enableDeferredFirmwareMemoryReclaim{builder.flags.deferredFirmwareMemoryReclaim},
          enableSeperateSamplerTextureSupport{builder.flags.enableSeperateSamplerTextureSupport},
          textureManager{vkCore} {
        NNVK_FILL_VERSIONED_STRUCT(Device);
    }

    Device::~Device() {}

    void Device::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    // Dumped from NVN 55.12
    static constexpr std::array<i32, 105> IntegerValues{{0x0, 0x0, 0xE, 0x10000, 0x100, 0x8, 0x10, 0x4, 0x10, 0x20, 0x10, 0x4, 0x4, 0x4, 0x10, 0x20, 0x20, 0x100, 0x100, 0x4, 0x8, 0x200, 0x100, 0x2, 0x100, 0x1000, 0x20000, 0x10, 0x600, 0x400, 0x40, 0x600, 0xFFFF, 0xFFFF, 0xFFFF, 0x8, 0x100000, 0x1000, 0x10, 0x1000, 0x1, 0x1, 0x1, 0x40000, 0xF, 0x800, 0x1, 0x1, 0x10, 0x5, 0x1, 0x8, 0x8, 0x4, 0x20, 0x80, 0x20, 0x80, 0x10000, 0x1, 0x4, 0x4000, 0x8000000, 0x800, 0x4000, 0x4000, 0x1, 0x1, 0x1, 0x1, 0x5, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x10, 0x400, 0x20, 0x1000, 0x10000, 0x10000, 0x100, 0x4000, 0x40000, 0x1000, 0x1, 0x4, 0x2, 0x0, 0x1000, 0x4000, 0x1000, 0x80, 0x20, 0xFFFF, 0x1, 0x20, 0x100, 0x10, 0xFFFF, 0xFFFF, 0x100, 0x3FFF}};
    void Device::GetInteger(ApiVersion version, DeviceInfo pname, i32 *v) const {
        if (static_cast<u32>(pname) >= IntegerValues.size())
            throw std::runtime_error(fmt::format("Unknown DeviceInfo value: {}", static_cast<u32>(pname)));

        if (pname == DeviceInfo::ApiMajorVersion)
            *v = version.major;
        else if (pname == DeviceInfo::ApiMinorVersion)
            *v = version.minor;
        else
            *v = IntegerValues[static_cast<u32>(pname)];
    }

    u64 Device::GetCurrentTimestampInNanoseconds() const {
        return static_cast<u64>(util::GetTimeNs());
    }

    void Device::SetIntermediateShaderCache(i32 i) {}

    TextureHandle Device::GetTextureHandle(i32 textureId, i32 samplerId) const {
        return GetSeparateSamplerHandle(samplerId) | GetSeparateTextureHandle(textureId);
    }

    TextureHandle Device::GetTexelFetchHandle(i32 textureId) const {
        return GetSeparateTextureHandle(textureId);
    }

    ImageHandle Device::GetImageHandle(i32 textureId) const {
        return GetSeparateTextureHandle(textureId);
    }

    void Device::InstallDebugCallback(const void *callback, void *callbackData, bool enable) {}

    DebugDomainId Device::GenerateDebugDomainId(const char *s) const {
        throw std::runtime_error("GenerateDebugDomainId is not implemented");
    }

    void Device::SetWindowOriginMode(WindowOriginMode mode) {
        windowOriginMode = mode;
    }

    void Device::SetDepthMode(DepthMode mode) {
        depthMode = mode;
    }

    // TODO ERRORS?
    bool Device::RegisterFastClearColor(const float *color, Format format) {
        return true;
    }

    bool Device::RegisterFastClearColori(const i32 *color, Format format) {
        return true;
    }

    bool Device::RegisterFastClearColorui(const u32 *color, Format format) {
        return true;
    }

    bool Device::RegisterFastClearDepth(f32 f) {
        return true;
    }

    WindowOriginMode Device::GetWindowOriginMode() const {
        return windowOriginMode;
    }

    DepthMode Device::GetDepthMode() const {
        return depthMode;
    }

    u64 Device::GetTimestampInNanoseconds(const CounterData *counterData) const {
        return (counterData->timestamp * 625) / 384;
    }

    void Device::ApplyDeferredFinalizes(i32 i) {
        throw std::runtime_error("ApplyDeferredFinalizes is not implemented");
    }

    void Device::FinalizeCommandHandle(CommandHandle handles) {
        throw std::runtime_error("FinalizeCommandHandle is not implemented");
    }

    void Device::WalkDebugDatabase(DebugObjectType type, const void *callback, void *callbackData) const {}

    SeparateTextureHandle Device::GetSeparateTextureHandle(i32 textureId) const {
        return static_cast<SeparateTextureHandle>(textureId & ((1 << 20) - 1));
    }

    SeparateSamplerHandle Device::GetSeparateSamplerHandle(i32 samplerId) const {
        return static_cast<SeparateSamplerHandle>(samplerId << 20);
    }

    bool Device::IsExternalDebuggerAttached() const {
        return false;
    }

    /* Wrappers */
    bool Context::DeviceInitialize(Device *device, const DeviceBuilder *builder) {
        new (device) Device(apiVersion, *builder, vkCore);
        return true;
    }

    void Context::DeviceFinalize(Device *device) {
        device->~Device();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, SetDebugLabel, const char *)

    void Context::DeviceGetInteger(Device *device, DeviceInfo pname, i32 *v) {
        device->GetInteger(apiVersion, pname, v);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_0(u64, Device, GetCurrentTimestampInNanoseconds)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, SetIntermediateShaderCache, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(TextureHandle, Device, GetTextureHandle, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(TextureHandle, Device, GetTexelFetchHandle, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(ImageHandle, Device, GetImageHandle, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Device, InstallDebugCallback, const void *, void *, bool)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(DebugDomainId, Device, GenerateDebugDomainId, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, SetWindowOriginMode, WindowOriginMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, SetDepthMode, DepthMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, Device, RegisterFastClearColor, const float *, Format)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, Device, RegisterFastClearColori, const i32 *, Format)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, Device, RegisterFastClearColorui, const u32 *, Format)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(bool, Device, RegisterFastClearDepth, f32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(WindowOriginMode, Device, GetWindowOriginMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(DepthMode, Device, GetDepthMode)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(u64, Device, GetTimestampInNanoseconds, const CounterData *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, ApplyDeferredFinalizes, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Device, FinalizeCommandHandle, CommandHandle)
    NNVK_CONTEXT_WRAP_TRIVIAL_3(void, Device, WalkDebugDatabase, DebugObjectType, const void *, void *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(SeparateTextureHandle, Device, GetSeparateTextureHandle, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(SeparateSamplerHandle, Device, GetSeparateSamplerHandle, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(bool, Device, IsExternalDebuggerAttached)
    /* End wrappers */
}