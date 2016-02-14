#pragma once

#include "base/Platform.h"

namespace SocketOps
{
    socket_t    CreateSocket();
    void        CloseSocket(socket_t fd);
    int         Connect(socket_t fd, const struct sockaddr *sa);
    bool        BindSocket(socket_t fd, const struct sockaddr *sa);
    bool        Listen(socket_t fd);
    socket_t    Accept(socket_t fd, struct sockaddr *addr);
    bool        SetSocketNoneBlocking(socket_t fd);
    int         ReuseListenSocket(socket_t fd);
    int         CreateSocketPair(socket_t fdPair[2]);
}