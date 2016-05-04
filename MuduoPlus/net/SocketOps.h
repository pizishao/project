#pragma once

#include <stdint.h>

#include "base/define.h"

namespace SocketOps
{
    inline uint64_t hostToNetwork64(uint64_t host64)
    {
        //return htonll(host64);
        return host64;
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
        //return ntohll(net64);
        return net64;
    }

    inline uint32_t networkToHost32(uint32_t net32)
    {
        return ntohl(net32);
    }

    inline uint16_t networkToHost16(uint16_t net16)
    {
        return ntohs(net16);
    }

    socket_t    createSocket();
    void        closeSocket(socket_t fd);
    int         connect(socket_t fd, const struct sockaddr *sa);
    bool        bindSocket(socket_t fd, const struct sockaddr *sa);
    bool        listen(socket_t fd);
    socket_t    accept(socket_t fd, struct sockaddr *addr);
    int         send(socket_t fd, const void* buff, int count);
    int         secv(socket_t fd, char *buff, int count);

    bool        setSocketNoneBlocking(socket_t fd);
    void        setKeepAlive(socket_t fd, bool on);
    void        shutdownWrite(int fd);
    void        setTcpNoDelay(int fd, bool on);
    int         reuseListenSocket(socket_t fd);
    int         createSocketPair(socket_t fdPair[2]);
    sockaddr_in getPeerAddr(int sockfd);
    sockaddr_in getLocalAddr(int sockfd);
    int         getSocketError(socket_t fd);

}