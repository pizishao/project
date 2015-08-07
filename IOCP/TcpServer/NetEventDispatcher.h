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
	BlockingQueue<NetEvent> m_NetEventQueue;
	bool m_bQuit;
};