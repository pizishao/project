#pragma once

#include "Net.h"

class InetAddress;

namespace Sock
{
	uint32_t HostToNetwork32(std::string sIp);
	uint16_t HostToNetwork16(uint16_t port);

	int  CreateSocket();
	void CloseFd(SOCKET &sk);
	bool BindSocket(SOCKET sk, const InetAddress &inetAddress);
	bool Listen(SOCKET sk);
	bool Connect(SOCKET sk, const InetAddress &inetAddress);
	//int  Accept(SOCKET sk, InetAddress &inetAddress);
	//ssize_t	Send(int sk, const void *buf, size_t len);
	//ssize_t Recv(int sk, void *buf, size_t len);
	bool SetSockopt(SOCKET sk, int optname, const char *optval, int optlen);
	//bool SetSockNoneBlock(int sk);
	void GetPeerAddr(SOCKET sk, InetAddress &inetAddress);
	int  GetErrCode();
	std::string GetErrText();
}
