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
		memset(&m_addr, 0, sizeof(m_addr));  

		m_addr.sin_family = AF_INET;  
		m_addr.sin_addr.s_addr = Sock::HostToNetwork32(ip);  
		m_addr.sin_port = Sock::HostToNetwork16(port);  
	}

	InetAddress(const struct sockaddr_in& addr)
		: m_addr(addr)
	{

	}

public:
	const struct sockaddr_in& GetSockAddrInet() const { return m_addr; }
	void GetSockAddrInet(const struct sockaddr_in& addr) { m_addr = addr; }

	void SetSockAddrInet(const struct sockaddr_in& addr) {m_addr = addr;}

	uint32_t IpNetEndian() const { return m_addr.sin_addr.s_addr; }
	uint16_t PortNetEndian() const { return m_addr.sin_port; }

	std::string	IpString() const { std::string s(inet_ntoa(m_addr.sin_addr)); return s;}
	uint16_t    PortHostEndian() const {return  ntohs(m_addr.sin_port); }

private:
	struct sockaddr_in m_addr;
};
