#pragma once

#include "Net.h"
#include "BlockingQueue.h"

class NetEventDispatcher
{
	friend class TcpServer;

public:
	NetEventDispatcher();
	~NetEventDispatcher(){}

public:	
	void Dispatch();	

private:
	void AddEvent(const NetEvent &event);
	void Stop();

private:
	BlockingQueue<NetEvent> m_netEventQueue;
	bool m_bQuit;
};