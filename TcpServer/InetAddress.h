#pragma once

#include <stdint.h>
#include <string>

#include "Net.h"
#include "Sock.h"

class InetAddress
{
public:
	InetAddress(){}

	InetAddress(std::string ip, uint16_t port)
	{
		memset(&addr_, 0, sizeof(addr_));  

		addr_.sin_family = AF_INET;  
		addr_.sin_addr.s_addr = Sock::HostToNetwork32(ip);  
		addr_.sin_port = Sock::HostToNetwork16(port);  
	}

	InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{

	}

public:
	const struct sockaddr_in& GetSockAddrInet() const { return addr_; }
	void GetSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

	void SetSockAddrInet(const struct sockaddr_in& addr) {addr_ = addr;}

	uint32_t IpNetEndian() const { return addr_.sin_addr.s_addr; }
	uint16_t PortNetEndian() const { return addr_.sin_port; }

	std::string	IpString() const { std::string s(inet_ntoa(addr_.sin_addr)); return s;}
	uint16_t PortHostEndian() const {return  ntohs(addr_.sin_port); }

private:
	struct sockaddr_in addr_;
};
