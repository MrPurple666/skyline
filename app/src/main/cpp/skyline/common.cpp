// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <dlfcn.h>
#include <adrenotools/driver.h>
#include "common/settings.h"
#include "common.h"
#include "nce.h"
#include "soc.h"
#include "gpu.h"
#include "audio.h"
#include "input.h"
#include "os.h"
#include "hle/nnvk/nnvk.h"
#include "hle/nnvk/logging.h"
#include "kernel/types/KProcess.h"

namespace nnvk {
    static skyline::Logger::LogLevel ConvertLogLevel(Logger::LogLevel level) {
        switch (level) {
            case Logger::LogLevel::Debug:
                return skyline::Logger::LogLevel::Debug;
            case Logger::LogLevel::Info:
                return skyline::Logger::LogLevel::Info;
            case Logger::LogLevel::Warn:
                return skyline::Logger::LogLevel::Warn;
            case Logger::LogLevel::Error:
                return skyline::Logger::LogLevel::Error;
        }
    }

    void Logger::Write(Logger::LogLevel level, const std::string &str) {
        skyline::Logger::Write(ConvertLogLevel(level), str);
    }

    bool Logger::IsEnabled(Logger::LogLevel level) {
        return skyline::Logger::configLevel >= ConvertLogLevel(level);
    }
}
namespace nnvk::util {
    i64 GetTimeNs() {
        return skyline::util::GetTimeNs();
    }
}

namespace skyline {
    static std::pair<PFN_vkGetInstanceProcAddr, void *> LoadVulkanDriver(kernel::OS &os, Settings &settings) {
        void *libvulkanHandle{};

        void *importHandle{};
        // If the user has selected a custom driver, try to load it
        if (!(*settings.gpuDriver).empty()) {
            libvulkanHandle = adrenotools_open_libvulkan(
                RTLD_NOW,
                ADRENOTOOLS_DRIVER_FILE_REDIRECT | ADRENOTOOLS_DRIVER_CUSTOM | ADRENOTOOLS_DRIVER_GPU_MAPPING_IMPORT,
                nullptr, // We require Android 10 so don't need to supply
                os.nativeLibraryPath.c_str(),
                (os.privateAppFilesPath + "gpu_drivers/" + *settings.gpuDriver + "/").c_str(),
                (*settings.gpuDriverLibraryName).c_str(),
                (os.publicAppFilesPath + "gpu/vk_file_redirect/").c_str(),
                &importHandle
            );

            if (!libvulkanHandle) {
                char *error = dlerror();
                Logger::Warn("Failed to load custom Vulkan driver {}/{}: {}", *settings.gpuDriver, *settings.gpuDriverLibraryName, error ? error : "");
            }
        }

        if (!libvulkanHandle) {
            libvulkanHandle = adrenotools_open_libvulkan(
                RTLD_NOW,
                ADRENOTOOLS_DRIVER_FILE_REDIRECT | ADRENOTOOLS_DRIVER_GPU_MAPPING_IMPORT,
                nullptr, // We require Android 10 so don't need to supply
                os.nativeLibraryPath.c_str(),
                nullptr,
                nullptr,
                (os.publicAppFilesPath + "gpu/vk_file_redirect/").c_str(),
                &importHandle
            );

            if (!libvulkanHandle) {
                char *error = dlerror();
                Logger::Warn("Failed to load builtin Vulkan driver: {}", error ? error : "");
            }

            if (!libvulkanHandle)
                libvulkanHandle = dlopen("libvulkan.so", RTLD_NOW);
        }

        return {reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkanHandle, "vkGetInstanceProcAddr")), importHandle};
    }

    DeviceState::DeviceState(kernel::OS *pOs, std::shared_ptr<JvmManager> jvmManager, std::shared_ptr<Settings> pSettings)
        : os(pOs), jvm(std::move(jvmManager)), settings(std::move(pSettings)) {
        auto [vkGetInstanceProcAddr, adrenotoolsImportHandle]{LoadVulkanDriver(*os, *settings)};
        nnvkContext = std::make_unique<nnvk::Context>(vkGetInstanceProcAddr, adrenotoolsImportHandle, *settings->validationLayer);
        // We assign these later as they use the state in their constructor and we don't want null pointers
        gpu = std::make_shared<gpu::GPU>(*this, vkGetInstanceProcAddr, adrenotoolsImportHandle);
        soc = std::make_shared<soc::SOC>(*this);
        audio = std::make_shared<audio::Audio>(*this);
        nce = std::make_shared<nce::NCE>(*this);
        scheduler = std::make_shared<kernel::Scheduler>(*this);
        input = std::make_shared<input::Input>(*this);
    }

    DeviceState::~DeviceState() {
        if (process)
            process->ClearHandleTable();
    }
}
