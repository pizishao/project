#pragma once

#include "Net.h"
#include "BlockingQueue.h"

class NetEventDispatcher
{
public:
	NetEventDispatcher(){}
	~NetEventDispatcher(){}

public:
	void AddEvent(const NetEvent &event);
	void Dispatch();

private:
	BlockingQueue<NetEvent> m_NetEventQueue;
};