#pragma once

#include "Net.h"
#include "BlockingQueue.h"

class NetEventDispatcher
{
public:
	NetEventDispatcher();
	~NetEventDispatcher(){}

public:
	void AddEvent(const NetEvent &event);
	void Dispatch();
	void Stop();

private:
	BlockingQueue<NetEvent> m_netEventQueue;
	bool m_bQuit;
};