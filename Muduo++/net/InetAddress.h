#pragma once

#include <string>
#include <stdint.h>

#include "base/LinuxWin.h"

namespace MuduoPlus
{

    class InetAddress
    {
    public:
        InetAddress(){}

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

    public:
        struct sockaddr_in& GetSockAddr() { return addr_; }
        void                SetSockAddr(const struct sockaddr_in& addr) { addr_ = addr; }

        uint32_t            IpNetEndian() const { return addr_.sin_addr.s_addr; }
        uint16_t            PortNetEndian() const { return addr_.sin_port; }

        std::string	        IpString() const { std::string s(inet_ntoa(addr_.sin_addr)); return s; }
        uint16_t            PortHostEndian() const { return  ntohs(addr_.sin_port); }

    private:
        struct sockaddr_in addr_;
    };
}
