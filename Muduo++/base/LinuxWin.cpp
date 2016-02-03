#include <string.h>

#include "LinuxWin.h"

int GetErrorCode()
{
#ifdef WIN32
    return GetLastError();
#else
    return errno;
#endif
}

std::string GetErrorText(int errcode)
{
#ifdef WIN32
    LPVOID buffer;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        errcode,
        0,
        (LPTSTR)&buffer,
        0,
        nullptr);

    return (char *)buffer;
#else
    return (strerror(errcode));
#endif    
}

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

bool Connect(socket_t fd, const struct sockaddr *sa)
{
    return false;
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
