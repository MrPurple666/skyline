#include <new>
#include <stdexcept>
#include <fmt/format.h>
#include "logging.h"
#include "types.h"
#include "nnvk.h"
#include "vkcore.h"
#include "window.h"

namespace nnvk {
    WindowBuilder::WindowBuilder(ApiVersion version) {
        NNVK_FILL_VERSIONED_STRUCT(WindowBuilder);
    }

    void WindowBuilder::SetDevice(Device *pDevice) {
        device = pDevice;
    }

    void WindowBuilder::SetNativeWindow(NativeWindow pNativeWindow) {
        nativeWindow = pNativeWindow;
    }

    void WindowBuilder::SetTextures(i32 count, Texture **pTextures) {
        numTextures = count;
        textures = pTextures;
    }

    void WindowBuilder::SetPresentInterval(i32 pPresentInterval) {
        presentInterval = pPresentInterval;
    }

    void WindowBuilder::SetNumActiveTextures(i32 pNumActiveTextures) {
        numActiveTextures = pNumActiveTextures;
    }

    i32 WindowBuilder::GetNumActiveTextures() const {
        return numActiveTextures;
    }

    const Device *WindowBuilder::GetDevice() const {
        return device;
    }

    i32 WindowBuilder::GetNumTextures() const {
        return numTextures;
    }

    const Texture *WindowBuilder::GetTexture(i32 i) const {
        if (i >= numTextures)
            throw std::out_of_range(fmt::format("Texture index {} out of range", i));

        return textures[i];
    }

    NativeWindow WindowBuilder::GetNativeWindow() const {
        return nativeWindow;
    }

    i32 WindowBuilder::GetPresentInterval() const {
        return presentInterval;
    }

    /* Wrappers */
    void Context::WindowBuilderSetDefaults(WindowBuilder *builder) {
        new (builder) WindowBuilder(apiVersion);
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, WindowBuilder, SetDevice, Device *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, WindowBuilder, SetNativeWindow, NativeWindow)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(void, WindowBuilder, SetTextures, i32, Texture **)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, WindowBuilder, SetPresentInterval, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, WindowBuilder, SetNumActiveTextures, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, WindowBuilder, GetNumActiveTextures)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(const Device *, WindowBuilder, GetDevice)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, WindowBuilder, GetNumTextures)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(const Texture *, WindowBuilder, GetTexture, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(NativeWindow, WindowBuilder, GetNativeWindow)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, WindowBuilder, GetPresentInterval)
    /* End wrappers */

    void Window::PresentTexture(std::unique_ptr<vkcore::BinarySemaphore> semaphore, i32 textureIndex) {}

    Window::Window(ApiVersion version, const WindowBuilder &builder)
        : device{builder.device},
          presentInterval{builder.presentInterval},
          nativeWindow{builder.nativeWindow},
          textures{builder.textures, builder.textures + builder.numTextures} {
        NNVK_FILL_VERSIONED_STRUCT(Window);
    }

    Window::~Window() {}

    void Window::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    WindowAcquireTextureResult Window::AcquireTexture(Sync *textureAvailableSync, i32 *textureIndex) {
        *textureIndex = nextTextureIndex;
        nextTextureIndex = (nextTextureIndex + 1) % static_cast<i32>(textures.size());

        vkcore::BinarySemaphore semaphore{device->vkCore.device.createSemaphore({})};
        // DO PRESENT

        device->vkCore.scheduler.GenAndQueueOperation([&]() -> vkcore::CommandScheduler::Operation {
            u64 signalTimelineValue{device->vkCore.scheduler.IncrTimeline()};
            textureAvailableSync->Fence(signalTimelineValue);

            return vkcore::CommandScheduler::SyncOperation{
                .signalTimelineValue = signalTimelineValue,
                .waitStageMask = vk::PipelineStageFlagBits2KHR::eAllCommands, // TODO: check, maybe only image stuff
                .waitBinarySemaphore = std::move(semaphore),
            };
        });

        return WindowAcquireTextureResult::Success;
    }

    NativeWindow Window::GetNativeWindow() const {
        return nativeWindow;
    }

    i32 Window::GetPresentInterval() const {
        return presentInterval;
    }

    void Window::SetPresentInterval(i32 pPresentInterval) {
        presentInterval = pPresentInterval;
    }

    void Window::SetCrop(i32 x, i32 y, i32 w, i32 h) {
        Logger::Error("SetCrop not implemented: x: {}, y: {}, w: {}, h: {}", x, y, w, h);
    }

    void Window::GetCrop(Rectangle *rectangle) {
        throw std::runtime_error("Window::GetCrop not implemented");
    }

    void Window::SetNumActiveTextures(i32 pNumActiveTextures) {
        numActiveTextures = pNumActiveTextures;
    }

    i32 Window::GetNumActiveTextures() const {
        return numActiveTextures;
    }

    i32 Window::GetNumTextures() const {
        return static_cast<i32>(textures.size());
    }

    /* Wrappers */
    bool Context::WindowInitialize(Window *pool, const WindowBuilder *builder) {
        new (pool) Window(apiVersion, *builder);
        return true;
    }

    void Context::WindowFinalize(Window *pool) {
        pool->~Window();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Window, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(WindowAcquireTextureResult, Window, AcquireTexture, Sync *, i32 *)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(NativeWindow, Window, GetNativeWindow)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Window, GetPresentInterval)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Window, SetPresentInterval, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_4(void, Window, SetCrop, i32, i32, i32, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Window, GetCrop, Rectangle *)
    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Window, SetNumActiveTextures, i32)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Window, GetNumActiveTextures)
    NNVK_CONTEXT_WRAP_TRIVIAL_0(i32, Window, GetNumTextures)
    /* End wrappers */
}