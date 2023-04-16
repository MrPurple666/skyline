// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <cxxabi.h>
#include "nvn.h"
#include "nnvk/nnvk.h"
#include <nce/guest.h>
#include "symbol_hooks.h"

namespace skyline::hle {
    std::string Demangle(const std::string_view mangledName) {
        int status{};
        size_t length{};
        std::unique_ptr<char, decltype(&std::free)> demangled{abi::__cxa_demangle(mangledName.data(), nullptr, &length, &status), std::free};
        return std::string{status == 0 ? std::string_view{demangled.get()} : mangledName};
    }

    HookedSymbol::HookedSymbol(std::string pName, HookType hook) : name{std::move(pName)}, prettyName{Demangle(name)}, hook{std::move(hook)} {}

    thread_local const char *RequestedProc{};

    namespace hooks {
        void NvnBootstrapLoaderEntry(const DeviceState &state, const HookedSymbol &symbol) {
            RequestedProc = reinterpret_cast<const char *>(state.ctx->gpr.x0);
        }

        void NvnBootstrapLoaderExit(const DeviceState &state, const HookedSymbol &symbol) {
            if (!strncmp(RequestedProc, "nvnDeviceGetProcAddress", 23)) {
                using NvnDeviceGetProcAddr = u64 (*)(void *, const char *);
                using NvnDeviceGetInteger = void (*)(void *, int, int *);

                auto nvnDeviceGetProcAddress{reinterpret_cast<NvnDeviceGetProcAddr>(state.ctx->gpr.x0)};
                auto nvnDeviceGetInteger{reinterpret_cast<NvnDeviceGetInteger>(
                                             nvnDeviceGetProcAddress(nullptr, "nvnDeviceGetInteger"))};
                int apiMajor{}, apiMinor{};
                nvnDeviceGetInteger(nullptr, 0, &apiMajor);
                nvnDeviceGetInteger(nullptr, 1, &apiMinor);
                Logger::Info("Game NVN version: {}.{}", apiMajor, apiMinor);
                state.nnvkContext->SetApiVersion(apiMajor, apiMinor);
            }

            Logger::Info("Loading proc: {}", RequestedProc);
            state.ctx->gpr.x0 = reinterpret_cast<u64>(NvnBootstrapLoader(RequestedProc));
        }
    }
}
