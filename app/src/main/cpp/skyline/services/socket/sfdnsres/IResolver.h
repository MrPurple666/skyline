// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/serviceman.h>

namespace skyline::service::socket {
    /**
     * @url https://switchbrew.org/wiki/Sockets_services#sfdnsres
     */
    class IResolver : public BaseService {
      public:
        IResolver(const DeviceState &state, ServiceManager &manager);

        Result GetAddrInfoRequest(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

        SERVICE_DECL(
          SFUNC(0x6, IResolver, GetAddrInfoRequest)
        )
    };
}
