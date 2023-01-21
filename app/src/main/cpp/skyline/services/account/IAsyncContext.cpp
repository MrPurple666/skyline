// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IAsyncContext.h"

namespace skyline::service::account {
    IAsyncContext::IAsyncContext(const DeviceState &state, ServiceManager &manager)
        : BaseService(state, manager),
          systemEvent(std::make_shared<kernel::type::KEvent>(state, true)) {}

    Result IAsyncContext::GetSystemEvent(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto handle{state.process->InsertItem(systemEvent)};    // Probable que mal
        Logger::Debug("Accumulated Suspended Tick Event Handle: 0x{:X}", handle);

        response.copyHandles.push_back(handle);
        return {};

        /*
            RegisterService: Service has been registered: "account::IAsyncContext" (0x2)
            SignalProcessWideKey: Signalling 0x848FA3FCC for 1 waiters
            Service: IAsyncContext::GetSystemEvent
            GetSystemEvent: Accumulated Suspended Tick Event Handle: 0xD029
            SignalProcessWideKey: Signalling 0x848FA3FCC for 1 waiters  -> presignaled(false)
            WaitSynchronization: Waiting on 0xD029 for -1ns
         */
    }

    Result IAsyncContext::HasDone(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u8>(true);
        return {};
    }

    Result IAsyncContext::GetResult(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }
}
