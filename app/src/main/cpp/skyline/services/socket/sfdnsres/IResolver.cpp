// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IResolver.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

namespace skyline::service::socket {
    IResolver::IResolver(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}

    Result IResolver::GetAddrInfoRequest(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        // https://www.ibm.com/docs/en/i/7.1?topic=ssw_ibm_i_71/apis/getaddrinfo.html
        // https://www.ibm.com/docs/en/zos/2.1.0?topic=SSLTBW_2.1.0/com.ibm.zos.v2r1.halc001/ipciccla_getaddrinfo_parm.html
        struct Parameters {
            u8 use_nsd_resolve;
            u32 cancelHandle;
            u64 process_id;
        };

        //Must be in network byte order
        // https://www.ibm.com/docs/en/zos/2.4.0?topic=hosts-network-byte-order
        struct addrinfo_switch {
            u32	magic;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
            u32	ai_flags;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
            u32	ai_family;	/* PF_xxx */
            u32	ai_socktype;	/* SOCK_xxx */
            u32	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
            socklen_t ai_addrlen;	/* length of ai_addr */
            struct	sockaddr *ai_addr;	/* binary address */
            char	*ai_canonname;	/* canonical name for hostname */
            u32 sentinel_value;
        };

        auto hostname{request.inputBuf.at(0).as_string(true)};
        auto service{request.inputBuf.at(1).as_string(true)};

        const auto parameters = request.Pop<Parameters>();
        Logger::Warn("Called with ignored parameters: use_nsd_resolve={}, cancelHandle={}, process_id={}",
                     parameters.use_nsd_resolve, parameters.cancelHandle, parameters.process_id);

        addrinfo* addrinfo;
        ::getaddrinfo(hostname.data(), service.data(), nullptr, &addrinfo);

        //addrinfo_switch toRet {0xBEEFCAFE, addrinfo->ai_flags, addrinfo->ai_family, addrinfo->ai_socktype,
        //                       addrinfo->ai_protocol, addrinfo->ai_addrlen, addrinfo->ai_addr, addrinfo->ai_canonname, 0};
        std::vector<addrinfo_switch> toRet;
        toRet.push_back({htonl(0xBEEFCAFE), htonl(addrinfo->ai_flags), htonl(addrinfo->ai_family), htonl(addrinfo->ai_socktype),
                         htonl(addrinfo->ai_protocol), htonl(addrinfo->ai_addrlen), addrinfo->ai_addr, addrinfo->ai_canonname, 0});
        toRet[0].ai_addr->sa_family >>= 8;
        u8 ip[4]{toRet[0].ai_addr->sa_data[2],toRet[0].ai_addr->sa_data[3], toRet[0].ai_addr->sa_data[4], toRet[0].ai_addr->sa_data[5]};

        Logger::Warn("Host '{}:{}' resolved to IPv4 address '{}.{}.{}.{}'", hostname, service, ip[0], ip[1], ip[2], ip[3]);
        //The list should be terminated with a sentinel four-byte zero value.
        request.outputBuf.at(0).copy_from(toRet);
        response.Push<u32>(0); //result
        response.Push<u32>(errno);  // Error code
        response.Push<u32>(toRet.size());
        return {};
    }
}
