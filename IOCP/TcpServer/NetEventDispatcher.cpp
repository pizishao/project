#include "NetEventDispatcher.h"

NetEventDispatcher::NetEventDispatcher()
{
	m_bQuit = false;
}

void NetEventDispatcher::AddEvent( const NetEvent &event )
{
	m_netEventQueue.put(event);
}

void NetEventDispatcher::Dispatch()
{
	while (!m_bQuit)
	{
		std::deque<NetEvent> netEventQueue;

		m_netEventQueue.take(netEventQueue); //»á×èÈû

		if (m_bQuit)
		{
			return;
		}

		for (auto & eachEvent : netEventQueue)
		{
			if (eachEvent.m_eventType == NetEvent::en_Connect)
			{
				if (eachEvent.m_connFunctor)
				{
					eachEvent.m_connFunctor(eachEvent.m_llClientHandle);
				}
			}
			else if (eachEvent.m_eventType == NetEvent::en_Msg)
			{
				if (eachEvent.m_msgFunctor)
				{
					Packet &pkg = *eachEvent.m_pktPtr;

					if (pkg.size() > 0)
					{
						eachEvent.m_msgFunctor(eachEvent.m_llClientHandle, &pkg[0], pkg.size());
					}					
				}
			}
			else if (eachEvent.m_eventType == NetEvent::en_Close)
			{
				if (eachEvent.m_closeFunctor)
				{
					eachEvent.m_closeFunctor(eachEvent.m_llClientHandle);
				}
			}
            else if (eachEvent.m_eventType == NetEvent::en_Timer)
            {
                if (eachEvent.m_timerCallbackFunctor)
                {
                    eachEvent.m_timerCallbackFunctor();
                }
            }
		}
	}
}

void NetEventDispatcher::Stop()
{
	m_bQuit = true;
}

