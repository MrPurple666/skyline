// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)
// Copyright © 2020 yuzu Emulator Project (https://github.com/yuzu-emu/)

#include <input.h>
#include <input/npad.h>
#include <services/applet/common_arguments.h>
#include <services/am/storage/ObjIStorage.h>
#include "offline_web_applet.h"

namespace skyline::applet {
    OfflineWebApplet::OfflineWebApplet(const DeviceState &state,
                                       service::ServiceManager &manager,
                                       std::shared_ptr<kernel::type::KEvent> onAppletStateChanged,
                                       std::shared_ptr<kernel::type::KEvent> onNormalDataPushFromApplet,
                                       std::shared_ptr<kernel::type::KEvent> onInteractiveDataPushFromApplet,
                                       service::applet::LibraryAppletMode appletMode)
        : IApplet{state, manager, std::move(onAppletStateChanged), std::move(onNormalDataPushFromApplet), std::move(onInteractiveDataPushFromApplet), appletMode} {}

    Result OfflineWebApplet::Start() {
        auto commonArg{PopNormalInput<service::applet::CommonArguments>()};
        auto argHeader{PopNormalInput<WebArgHeader>()};

        if ((commonArg.apiVersion >= 0x80000 && argHeader.shimKind == ShimKind::Web) || (commonArg.apiVersion >= 0x30000 && argHeader.shimKind == ShimKind::Share))
            Logger::Error("OfflineWeb TLV output is unsupported!");

        PushNormalDataAndSignal(std::make_shared<service::am::ObjIStorage<WebCommonReturnValue>>(state, manager, WebCommonReturnValue{
            .exitReason = WebExitReason::WindowClosed
        }));

        // Notify the guest that we've finished running
        onAppletStateChanged->Signal();
        return {};
    }

    Result OfflineWebApplet::GetResult() {
        return {};
    }

    void OfflineWebApplet::PushNormalDataToApplet(std::shared_ptr<service::am::IStorage> data) {
        PushNormalInput(data);
    }

    void OfflineWebApplet::PushInteractiveDataToApplet(std::shared_ptr<service::am::IStorage> data) {}
}
