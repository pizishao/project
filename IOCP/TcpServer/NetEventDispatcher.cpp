#include "NetEventDispatcher.h"

NetEventDispatcher::NetEventDispatcher()
{
	m_bQuit = false;
}

void NetEventDispatcher::AddEvent( const NetEvent &event )
{
	m_NetEventQueue.put(event);
}

void NetEventDispatcher::Dispatch()
{
	while (!m_bQuit)
	{
		std::deque<NetEvent> netEventQueue;

		m_NetEventQueue.take(netEventQueue); //»á×èÈû

		if (m_bQuit)
		{
			return;
		}

		for (auto & eventEntry : netEventQueue)
		{
			if (eventEntry.m_eventType == NetEvent::en_Connect)
			{
				if (eventEntry.m_connFunctor)
				{
					eventEntry.m_connFunctor(eventEntry.m_llClientHandle);
				}
			}
			else if (eventEntry.m_eventType == NetEvent::en_Msg)
			{
				if (eventEntry.m_msgFunctor)
				{
					Packet &pkg = *eventEntry.m_pkt;

					if (pkg.size() > 0)
					{
						eventEntry.m_msgFunctor(eventEntry.m_llClientHandle, &pkg[0], pkg.size());
					}					
				}
			}
			else if (eventEntry.m_eventType == NetEvent::en_Close)
			{
				if (eventEntry.m_closeFunctor)
				{
					eventEntry.m_closeFunctor(eventEntry.m_llClientHandle);
				}
			}
		}
	}
}

void NetEventDispatcher::Stop()
{
	m_bQuit = true;
}

