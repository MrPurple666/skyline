// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include "IClient.h"

namespace skyline::service::socket {
    IClient::IClient(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager) {}

    Result IClient::RegisterClient(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u32>(0);
        return {};
    }

    Result IClient::StartMonitoring(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u64 unknown{request.Pop<u64>()};
        Logger::Warn("Unknown: {}", unknown);
        return {};
    }

    Result IClient::Socket(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto domain{request.Pop<u32>()};
        auto type{request.Pop<u32>()};
        auto protocol{request.Pop<u32>()};

        Logger::Warn("Domain {}, Type {}, Protocol {}", domain, type, protocol);
        //https://jameshfisher.com/2017/02/27/socket-types/
        //https://students.mimuw.edu.pl/SO/Linux/Kod/include/linux/socket.h.html
        //Socket: Domain 2, Type 1, Protocol 6
        //        AF_INET SOCK_STREAM

        //int BSD_Socket = ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
        int BSD_Socket = ::socket(domain, type, protocol);

        response.Push<int>(BSD_Socket); // Socket descriptor
        response.Push<u32>(0);  // Error code

        return {};
    }

    Result IClient::Select(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Poll(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Recv(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        //https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-recv-receive-data-socket
        int socketFd{request.Pop<int>()};
        int socketFlags{request.Pop<int>()};

        ::recv(socketFd, request.outputBuf.at(0).data(), request.outputBuf.at(0).size(), socketFlags);

        response.Push<u32>(0); //result
        Logger::Warn("errno {}", errno);
        response.Push<u32>(errno);  // Error code
        return {};
    }

    Result IClient::RecvFrom(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Send(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::SendTo(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        // https://linux.die.net/man/2/sendto
        int socketFd{request.Pop<int>()};
        int socketFlags{request.Pop<int>()};

        span message = request.inputBuf.at(0);
        long result = ::sendto(socketFd, request.inputBuf.at(0).data(), request.inputBuf.at(0).size(), socketFlags, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

        response.Push<u32>(0); //result
        Logger::Warn("result {} errno {}", result, errno);
        response.Push<u32>(errno);  // Error code
        return {};
    }

    Result IClient::Accept(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Bind(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Connect(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        int socketFd{request.Pop<int>()};
        Logger::Warn("errno before {}", errno);
        // https://docs.freebsd.org/en/books/developers-handbook/sockets/  (sockaddr_in structure)
        sockaddr = request.inputBuf.at(0).as<sockaddr_in>();
        Logger::Warn("sin_family {} sin_port {}  sin_addr {}", sockaddr.sin_family, sockaddr.sin_port, sockaddr.sin_addr.s_addr);

        sockaddr.sin_family >>= 8;
        sockaddr.sin_port = (sockaddr.sin_port>>8) | (sockaddr.sin_port<<8);
        sockaddr.sin_addr.s_addr = htonl(sockaddr.sin_addr.s_addr);

        Logger::Warn("sin_family {} sin_port {}  sin_addr {}", sockaddr.sin_family, sockaddr.sin_port, sockaddr.sin_addr.s_addr);

        int result = ::connect(socketFd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

        response.Push<u32>(0); //result
        Logger::Warn("result {} errno {}", result, errno); // If result is 0, do not care about errno
        response.Push<u32>(errno);  // Error code
        return {};
    }

    Result IClient::Listen(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::SetSockOpt(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Shutdown(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::ShutdownAllSockets(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Write(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        return {};
    }

    Result IClient::Read(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u32>(0);
        response.Push<u32>(0);
        return {};
    }

    Result IClient::Close(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        int socketFd{request.Pop<int>()};
        ::close(socketFd);
        return {};
    }
}
