#pragma once

#include <stdint.h>

#include "base/Platform.hpp"

namespace MuduoPlus
{

    class InetAddress;

    struct SockException
    {
        SockException()
        {
            error_code = 0;
        }

        int error_code;
    };

    namespace Sock
    {
        uint32_t HostToNetwork32(std::string sIp);
        uint16_t HostToNetwork16(uint16_t port);

        socket_t    CreateSocket(bool exception = false);
        void        CloseFd(socket_t &fd);
        bool Bind(socket_t fd, const InetAddress &inetAddress);
        bool Listen(socket_t fd);
        bool Connect(socket_t fd, const InetAddress &inetAddress);
        //int  Accept(socket_t fd, InetAddress &inetAddress);
        //ssize_t	Send(int fd, const void *buf, size_t len);
        //ssize_t Recv(int fd, void *buf, size_t len);
        bool SetSockopt(socket_t fd, int optname, const char *optval, int optlen);
        //bool SetSockNoneBlock(int fd);
        void GetPeerAddr(socket_t fd, InetAddress &inetAddress);
        int  GetErrCode();
        std::string GetErrText();
    }
}