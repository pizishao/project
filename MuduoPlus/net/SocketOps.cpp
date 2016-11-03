#include "base/LinuxWin.h"
#include "SocketOps.h"
#include "base/Logger.h"
#include <string.h>

namespace SocketOps
{
    socket_t    createSocket()
    {
        socket_t fd = socket(AF_INET, SOCK_STREAM, 0);

        return  fd;
    }

    void closeSocket(socket_t fd)
    {
#ifndef WIN32
        close(fd);
#else
        closesocket(fd);
#endif
    }

    int connect(socket_t fd, const struct sockaddr *sa)
    {
        return ::connect(fd, sa, sizeof(*sa));
    }

    bool bindSocket(socket_t fd, const struct sockaddr *sa)
    {
        if(bind(fd, sa, sizeof(*sa)) < 0)
        {
            return false;
        }

        return true;
    }

    bool listen(socket_t fd)
    {
        if(::listen(fd, SOMAXCONN) < 0)
        {
            return false;
        }

        return true;
    }

    socket_t accept(socket_t fd, struct sockaddr *addr)
    {
        socklen_t   socklen = sizeof(*addr);
        socket_t    newFd = accept(fd, addr, &socklen);

        return      newFd;
    }

    int send(socket_t fd, const void* buff, int count)
    {
        return ::send(fd, (char *)buff, count, 0);
    }

    int secv(socket_t fd, char *buff, int count)
    {
        return recv(fd, buff, count, 0);
    }

    bool setSocketNoneBlocking(socket_t fd)
    {
#ifdef WIN32
        {
            u_long nonblocking = 1;

            if(ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR)
            {
                return false;
            }
        }
#else
        {
            int flags;

            if((flags = fcntl(fd, F_GETFL, NULL)) < 0)
            {
                return false;
            }

            if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
            {
                return false;
            }
        }
#endif

        return true;
    }

    void setKeepAlive(socket_t fd, bool on)
    {
        int val = on ? 1 : 0;

        if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&val, sizeof(val)) < 0)
        {
            assert(false);
        }
    }

    void shutdownWrite(int fd)
    {
        if(shutdown(fd, SHUT_WR) < 0)
        {
            assert(false);
        }
    }

    void setTcpNoDelay(int fd, bool on)
    {
        int optval = on ? 1 : 0;

        if(::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                        (char *)&optval, sizeof(optval)) < 0)
        {
            assert(false);
        }
    }

    int reuseListenSocket(socket_t fd)
    {
#ifndef WIN32
        int one = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&one,
                          (socklen_t)sizeof(one));
#else
        return 0;
#endif
    }

    int createSocketPair(socket_t fdPair[2])
    {
        if(!fdPair)
        {
            return false;
        }

#ifndef WIN32

        if(socketpair(AF_UNIX, SOCK_STREAM, 0, fdPair) >= 0)
        {
            setSocketNoneBlocking(fdPair[0]);
            setSocketNoneBlocking(fdPair[1]);

            return true;
        }

        return false;
#endif
        socket_t listener = -1;
        socket_t connector = -1;
        socket_t acceptor = -1;
        struct sockaddr_in listenAddr = {0};
        struct sockaddr_in connectAddr = {0};
        socklen_t size = 0;

        listener = socket(AF_INET, SOCK_STREAM, 0);

        if(listener < 0)
        {
            return false;
        }

        listenAddr.sin_family = AF_INET;
        listenAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        listenAddr.sin_port = 0;	/* kernel chooses port.	 */

        if(::bind(listener, (struct sockaddr *) &listenAddr,
                  sizeof(listenAddr)) < 0)
        {
            goto err;
        }

        if(::listen(listener, 1) < 0)
        {
            goto err;
        }

        connector = socket(AF_INET, SOCK_STREAM, 0);

        if(connector < 0)
        {
            goto err;
        }

        /* We want to find out the port number to connect to.  */
        size = sizeof(connectAddr);

        if(getsockname(listener, (struct sockaddr *) &connectAddr, &size) < 0)
        {
            goto err;
        }

        if(size != sizeof(connectAddr))
        {
            goto err;
        }

        if(connect(connector, (struct sockaddr *) &connectAddr,
                   sizeof(connectAddr)) < 0)
        {
            goto err;
        }

        size = sizeof(listenAddr);
        acceptor = accept(listener, (struct sockaddr *) &listenAddr, &size);

        if(acceptor < 0)
        {
            goto err;
        }

        if(size != sizeof(listenAddr))
        {
            goto err;
        }

        if(getsockname(connector, (struct sockaddr *) &connectAddr, &size) < 0)
        {
            goto err;
        }

        if(size != sizeof(connectAddr)
                || listenAddr.sin_family != connectAddr.sin_family
                || listenAddr.sin_addr.s_addr != connectAddr.sin_addr.s_addr
                || listenAddr.sin_port != connectAddr.sin_port)
        {
            goto err;
        }

        if(!setSocketNoneBlocking(connector))
        {
            goto err;
        }

        if(!setSocketNoneBlocking(acceptor))
        {
            goto err;
        }

        closeSocket(listener);
        fdPair[0] = connector;
        fdPair[1] = acceptor;

        return true;

err:

        if(listener > 0)
        {
            closeSocket(listener);
        }

        if(connector > 0)
        {
            closeSocket(connector);
        }

        if(acceptor > 0)
        {
            closeSocket(acceptor);
        }

        return false;
    }

    sockaddr_in getPeerAddr(int sockfd)
    {
        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof(peeraddr));

        socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));

        if(::getpeername(sockfd, (sockaddr*)(&peeraddr), &addrlen) < 0)
        {
            assert(false);
        }

        return peeraddr;
    }

    sockaddr_in getLocalAddr(int sockfd)
    {
        struct sockaddr_in localaddr = { 0 };
        socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);

        if(::getsockname(sockfd, (sockaddr *)(&localaddr), &addrlen) < 0)
        {
            assert(false);
        }

        return localaddr;
    }

    int getSocketError(socket_t fd)
    {
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);
#ifdef WIN32

        if(::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen) < 0)
#else
        if(::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
#endif
        {
            return GetLastErrorCode();
        }
        else
        {
            return optval;
        }
    }
}