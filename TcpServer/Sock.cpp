#include "Sock.h"
#include "InetAddress.h"

#include <string>

namespace Sock
{
	uint32_t HostToNetwork32(std::string sIp)
	{
		return ::inet_addr(sIp.c_str());
	}

	uint16_t HostToNetwork16( uint16_t port )
	{
		return ::htons(port);
	}

	int CreateSocket()
	{
		SOCKET fd = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

		return fd;
	}

	void CloseFd(SOCKET &sk)
	{
		if (sk != INVALID_SOCKET)
		{
			closesocket(sk);
            sk=INVALID_SOCKET;
		}
	}

	bool BindSocket( SOCKET sk, const InetAddress &inetAddress )
	{
		if (::bind(sk, (const sockaddr*)&inetAddress.GetSockAddrInet(), sizeof(inetAddress.GetSockAddrInet())) 
            == SOCKET_ERROR)
		{
			return false;
		}

		return true;
	}

	bool Listen( SOCKET sk )
	{
		if (::listen(sk, SOMAXCONN) == SOCKET_ERROR)
		{
			return false;
		}

		return true;
	}

	bool Connect( SOCKET sk, const InetAddress &inetAddress )
	{
		if(connect(sk, (struct sockaddr *)(&inetAddress.GetSockAddrInet()), sizeof(struct sockaddr)) < 0)
		{
			return false;
		}

		return true;
	}

	/*int Accept( SOCKET sk, InetAddress &inetAddress )
	{
		socklen_t len = sizeof(inetAddress.GetSockAddrInet());

		int32 iAcceptFd = ::accept(sk, (sockaddr *)&inetAddress.GetSockAddrInet(), &len);

		return iAcceptFd;
	}*/

	/*ssize_t	Send(int sk, const void *buf, size_t len)
	{
		return ::send(sk, buf, len, 0);
	}

	ssize_t Recv(int sk, void *buf, size_t len)
	{
		return ::recv(sk, buf, len, 0);
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

    bool SetSockopt( SOCKET sk, int optname, const char *optval, int optlen )
    {
        if (::setsockopt(sk,SOL_SOCKET,optname,optval,optlen) == SOCKET_ERROR )
        {
            return false;
        }

        return true;
    }

	/*bool SetSockNoneBlock( int sk )
	{
		int32 iOldOption = fcntl(sk, F_GETFL);
		int32 iNewOption = iOldOption | O_NONBLOCK;

		if (fcntl(sk,F_SETFL,iNewOption) < 0)
		{
			return false;
		}

		return true;
	}*/

	void GetPeerAddr( SOCKET sk, InetAddress &inetAddress )
	{
		struct sockaddr_in peerAddr;
		int len = sizeof(peerAddr);
		getpeername(sk, (struct sockaddr *)&peerAddr, &len);

		inetAddress.SetSockAddrInet(peerAddr);
	}

}