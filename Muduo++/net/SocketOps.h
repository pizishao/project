#pragma once

#include <stdint.h>

#include "base/Platform.h"

namespace SocketOps
{
    inline uint64_t hostToNetwork64(uint64_t host64)
    {
        return htonll(host64);
    }

    inline uint32_t hostToNetwork32(uint32_t host32)
    {
        return htonl(host32);
    }

    inline uint16_t hostToNetwork16(uint16_t host16)
    {
        return htons(host16);
    }

    inline uint64_t networkToHost64(uint64_t net64)
    {
        return ntohll(net64);
    }

    inline uint32_t networkToHost32(uint32_t net32)
    {
        return ntohl(net32);
    }

    inline uint16_t networkToHost16(uint16_t net16)
    {
        return ntohs(net16);
    }

    socket_t    CreateSocket();
    void        CloseSocket(socket_t fd);
    ResultCode  Connect(socket_t fd, const struct sockaddr *sa);
    bool        BindSocket(socket_t fd, const struct sockaddr *sa);
    bool        Listen(socket_t fd);
    socket_t    Accept(socket_t fd, struct sockaddr *addr);
    int         Recv(socket_t fd, char *buff, size_t count);

    bool        SetSocketNoneBlocking(socket_t fd);
    int         ReuseListenSocket(socket_t fd);
    int         CreateSocketPair(socket_t fdPair[2]);
}