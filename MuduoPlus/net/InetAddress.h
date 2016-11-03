#pragma once

#include <string>
#include <stdint.h>
#include <memory.h>

#include "base/LinuxWin.h"

namespace MuduoPlus
{

    class InetAddress
    {
    public:
        InetAddress()
        {
            memset(&addr_, 0, sizeof(addr_));
        }

        InetAddress(std::string ip, uint16_t port)
        {
            memset(&addr_, 0, sizeof(addr_));

            addr_.sin_family = AF_INET;
            addr_.sin_addr.s_addr = inet_addr(ip.c_str());
            addr_.sin_port = htons(port);
        }

        InetAddress(const struct sockaddr_in& addr)
            : addr_(addr)
        {

        }

        InetAddress(const struct sockaddr& addr)
        {
            addr_ = *(sockaddr_in*)&addr;
        }

    public:
        struct sockaddr_in& getSockAddrIn()
        {
            return addr_;
        }
        void                setSockAddrIn(const struct sockaddr_in& addr)
        {
            addr_ = addr;
        }

        struct sockaddr&    getSockAddr()
        {
            return *(sockaddr *)&addr_;
        }
        void                setSockAddr(const struct sockaddr& addr)
        {
            addr_ = *(sockaddr_in*)&addr;
        }

        uint32_t            addrIp() const
        {
            return addr_.sin_addr.s_addr;
        }
        uint16_t            addrPort() const
        {
            return addr_.sin_port;
        }

        std::string	        ip() const
        {
            std::string s(inet_ntoa(addr_.sin_addr));
            return s;
        }
        uint16_t            port() const
        {
            return  ntohs(addr_.sin_port);
        }

        std::string         toIpPort() const
        {
            char buff[100] = { 0 };
            snprintf(buff, 100, "%s:%u", ip().c_str(), port());
            return buff;
        }

    private:
        struct sockaddr_in addr_;
    };
}
