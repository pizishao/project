#include <string>
#include <exception>

#include "SocketOps.h"
#include "InetAddress.h"

namespace MuduoPlus
{
    namespace Sock
    {
        bool IsSockError(int value)
        {
#ifdef WIN32
            if (value != SOCKET_ERROR)
            {
                return true;
            } 
#else
            if (value >= 0)
            {
                return true;
            }
#endif
            return false;
        }

        uint32_t HostToNetwork32(std::string sIp)
        {
            return ::inet_addr(sIp.c_str());
        }

        uint16_t HostToNetwork16(uint16_t port)
        {
            return ::htons(port);
        }

        int CreateSocket(bool exception /*= false*/)
        {
            socket_t fd = socket(AF_INET, SOCK_STREAM, 0);

            if (exception && IsSockError(fd))
            {
                SockException ex;
                ex.error_code = GetErrorCode();
                throw ex;
            }

            return fd;
        }

        void CloseFd(socket_t &fd)
        {
#ifdef WIN32
            closesocket(fd);
#else
            close(fd);
#endif
        }

        bool Bind(socket_t fd, const InetAddress &inetAddress)
        {
            if (::bind(fd, (const sockaddr*)&inetAddress.GetSockAddrInet(), sizeof(inetAddress.GetSockAddrInet()))
                == SOCKET_ERROR)
            {
                return false;
            }

            return true;
        }

        bool Listen(socket_t fd)
        {
            if (::listen(fd, SOMAXCONN) == SOCKET_ERROR)
            {
                return false;
            }

            return true;
        }

        bool Connect(socket_t fd, const InetAddress &inetAddress)
        {
            if (connect(fd, (struct sockaddr *)(&inetAddress.GetSockAddrInet()), sizeof(struct sockaddr)) < 0)
            {
                return false;
            }

            return true;
        }

        /*int Accept( socket_t fd, InetAddress &inetAddress )
        {
        socklen_t len = sizeof(inetAddress.GetSockAddrInet());

        int32 iAcceptFd = ::accept(fd, (sockaddr *)&inetAddress.GetSockAddrInet(), &len);

        return iAcceptFd;
        }*/

        /*ssize_t	Send(int fd, const void *buf, size_t len)
        {
        return ::send(fd, buf, len, 0);
        }

        ssize_t Recv(int fd, void *buf, size_t len)
        {
        return ::recv(fd, buf, len, 0);
        }*/

        int GetErrCode()
        {
            return errno;
        }

        std::string GetErrText()
        {
            std::string sErrorText;

            DWORD error;
            error = GetLastError();
            LPVOID buffer;
            ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
                error,
                0,
                (LPTSTR)&buffer,
                0,
                nullptr);

            sErrorText = (char *)buffer;

            return sErrorText;
        }

        bool SetSockopt(socket_t fd, int optname, const char *optval, int optlen)
        {
            if (::setsockopt(fd, SOL_SOCKET, optname, optval, optlen) == SOCKET_ERROR)
            {
                return false;
            }

            return true;
        }

        /*bool SetSockNoneBlock( int fd )
        {
        int32 iOldOption = fcntl(fd, F_GETFL);
        int32 iNewOption = iOldOption | O_NONBLOCK;

        if (fcntl(fd,F_SETFL,iNewOption) < 0)
        {
        return false;
        }

        return true;
        }*/

        void GetPeerAddr(socket_t fd, InetAddress &inetAddress)
        {
            struct sockaddr_in peerAddr;
            int len = sizeof(peerAddr);
            getpeername(fd, (struct sockaddr *)&peerAddr, &len);

            inetAddress.SetSockAddrInet(peerAddr);
        }
    }
}