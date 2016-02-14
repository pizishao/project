#include "base/LinuxWin.h"
#include "SocketOps.h"
#include "base/Logger.h"

namespace SocketOps
{
    socket_t    CreateSocket()
    {
        socket_t fd = socket(AF_INET, SOCK_STREAM, 0);

        return  fd;
    }

    void CloseSocket(socket_t fd)
    {
#ifndef WIN32
        close(fd);
#else
        closesocket(fd);
#endif
    }

    int Connect(socket_t fd, const struct sockaddr *sa)
    {
        if (connect(fd, sa, sizeof(*sa)) >= 0)
        {
            return 1;
        }

        int e = GetErrorCode();
        if (ERR_CONNECT_RETRIABLE(e))
        {
            return 0;
        }

        if (ERR_CONNECT_REFUSED(e))
        {
            return 2;
        }
    }

    bool BindSocket(socket_t fd, const struct sockaddr *sa)
    {
        if (bind(fd, sa, sizeof(*sa)) < 0)
        {
            return false;
        }

        return true;
    }

    bool Listen(socket_t fd)
    {
        if (listen(fd, SOMAXCONN) < 0)
        {
            return false;
        }

        return true;
    }

    socket_t Accept(socket_t fd, struct sockaddr *addr)
    {
        socklen_t   socklen = sizeof(*addr);
        socket_t    newFd = accept(fd, addr, &socklen);

        return      newFd;
    }

    bool SetSocketNoneBlocking(socket_t fd)
    {
#ifdef WIN32
    {
        u_long nonblocking = 1;
        if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            return false;
        }
    }
#else
    {
        int flags;
        if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
            return false;
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            return false;
        }
    }
#endif

        return true;
    }

    int ReuseListenSocket(socket_t fd)
    {
#ifndef WIN32
        int one = 1;
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&one,
            (socklen_t)sizeof(one));
#else
        return 0;
#endif
    }

    int CreateSocketPair(socket_t fdPair[2])
    {
        if (!fdPair)
        {
            return -1;
        }

#ifndef WIN32
        return socketpair(AF_UNIX, SOCK_STREAM, protocol, fdPair);
#endif
        socket_t listener = -1;
        socket_t connector = -1;
        socket_t acceptor = -1;
        struct sockaddr_in listen_addr = {0};
        struct sockaddr_in connect_addr = {0};
        socklen_t size = 0;
        int saved_errno = -1;

        listener = socket(AF_INET, SOCK_STREAM, 0);
        if (listener < 0)
        {
            return -1;
        }

        listen_addr.sin_family = AF_INET;
        listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        listen_addr.sin_port = 0;	/* kernel chooses port.	 */
        if (bind(listener, (struct sockaddr *) &listen_addr,
            sizeof(listen_addr)) == -1)
        {
            goto err;
        }

        if (listen(listener, 1) == -1)
        {
            goto err;
        }

        connector = socket(AF_INET, SOCK_STREAM, 0);
        if (connector < 0)
        {
            goto err;
        }

        /* We want to find out the port number to connect to.  */
        size = sizeof(connect_addr);
        if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
        {
            goto err;
        }

        if (size != sizeof(connect_addr))
        {
            goto err;
        }

        if (connect(connector, (struct sockaddr *) &connect_addr,
            sizeof(connect_addr)) == -1)
        {
            goto err;
        }

        size = sizeof(listen_addr);
        acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
        if (acceptor < 0)
        {
            goto err;
        }

        if (size != sizeof(listen_addr))
        {
            goto err;
        }

        if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
        {
            goto err;
        }

        if (size != sizeof(connect_addr)
            || listen_addr.sin_family != connect_addr.sin_family
            || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
            || listen_addr.sin_port != connect_addr.sin_port)
        {
            goto err;
        }

        CloseSocket(listener);
        fdPair[0] = connector;
        fdPair[1] = acceptor;

        return 0;

err:
        if (listener != -1)
        {
            CloseSocket(listener);
        }

        if (connector != -1)
        {
            CloseSocket(connector);
        }

        if (acceptor != -1)
        {
            CloseSocket(acceptor);
        }

        return -1;
    }
}