// SPDX-License-Identifier: MPL-2.0
// Copyright © 2023 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <vector>
#include <memory>
#include "vkcore.h"
#include "types.h"
#include "versioning.h"
#include "device.h"

namespace nnvk {
    struct WindowFlags {
        bool noFragmentInterlock : 1;
        bool noZCull : 1;
        u32 _pad_ : 30;
    };
    static_assert(sizeof(WindowFlags) == 4);

    using NativeWindow = u64;

    class Texture;

    class WindowBuilder {
      private:
        friend class Window;


        Device *device{};
        NativeWindow nativeWindow{};
        i32 presentInterval{};
        i32 numTextures{};
        Texture **textures{};
        i32 numActiveTextures{};

      public:
        WindowBuilder(ApiVersion version);

        void SetDevice(Device *device);

        void SetNativeWindow(NativeWindow nativeWindow);

        void SetTextures(i32 count, Texture **pTextures);

        void SetPresentInterval(i32 pPresentInterval);

        void SetNumActiveTextures(i32 pNumActiveTextures);

        i32 GetNumActiveTextures() const;

        const Device *GetDevice() const;

        i32 GetNumTextures() const;

        const Texture *GetTexture(i32 i) const;

        NativeWindow GetNativeWindow() const;

        i32 GetPresentInterval() const;
    };
    NNVK_VERSIONED_STRUCT(WindowBuilder, 0x40);

    enum class WindowAcquireTextureResult : i32 {
        Success,
        NativeError
    };

    class Sync;

    struct Rectangle;
    class Window {
      private:
        friend class Queue;

        const char *debugLabel{};
        Device *device;
        i32 presentInterval;
        NativeWindow nativeWindow;
        std::vector<Texture *> textures;
        i32 nextTextureIndex; // TODO: real rpesent
        i32 numActiveTextures;

        void PresentTexture(std::unique_ptr<vkcore::BinarySemaphore> semaphore, i32 textureIndex);

      public:
        Window(ApiVersion version, const WindowBuilder &builder);

        ~Window();

        void SetDebugLabel(const char *label);

        WindowAcquireTextureResult AcquireTexture(Sync *textureAvailableSync, i32 *textureIndex);

        NativeWindow GetNativeWindow() const;

        i32 GetPresentInterval() const;

        void SetPresentInterval(i32 presentInterval);

        void SetCrop(i32 x, i32 y, i32 w, i32 h);

        void GetCrop(Rectangle *rectangle);

        void SetNumActiveTextures(i32 pNumActiveTextures);

        i32 GetNumActiveTextures() const;

        i32 GetNumTextures() const;
    };
    NNVK_VERSIONED_STRUCT(Window, 0x180);
}
