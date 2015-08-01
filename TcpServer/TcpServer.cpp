#include "TcpServer.h"

TcpServer::TcpServer()
{
    m_bQuit = false;
}

TcpServer::~TcpServer()
{

}

void TcpServer::AddCloseNetEvent(int64_t llClientHandle)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Close;
    netEvent.m_llClientHandle=llClientHandle;

    m_NetEventQueue.put(netEvent);
}

void TcpServer::AddMsgNetEvent(int64_t llClientHandle, std::shared_ptr<Packet> &pkt)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Msg;
    netEvent.m_llClientHandle=llClientHandle;
    netEvent.m_pkt=pkt;

    m_NetEventQueue.put(netEvent);
}

void TcpServer::AddConnNetEvent(int64_t llClientHandle)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Connect;
    netEvent.m_llClientHandle=llClientHandle;

    m_NetEventQueue.put(netEvent);
}

void TcpServer::Dispatch()
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
                if (m_ConnCallbackFunctor)
                {
                    m_ConnCallbackFunctor(eventEntry.m_llClientHandle);
                }
            }
            else if (eventEntry.m_eventType == NetEvent::en_Msg)
            {
                if (m_MsgCallbackFunctor)
                {
                    Packet &pkg = *eventEntry.m_pkt;

                    if (pkg.size() > 0)
                    {
                        m_MsgCallbackFunctor(eventEntry.m_llClientHandle, &pkg[0], pkg.size());
                    }					
                }
            }
            else if (eventEntry.m_eventType == NetEvent::en_Close)
            {
                if (m_CloseCallbackFunctor)
                {
                    m_CloseCallbackFunctor(eventEntry.m_llClientHandle);
                }
            }
        }
    }
}

void TcpServer::SetConnectionCallback(ConnectionCallback connCallbackFunctor)
{
    m_ConnCallbackFunctor = connCallbackFunctor;
}

void TcpServer::SetMessageCallback(MessageCallback msgCallbackFunctor)
{
    m_MsgCallbackFunctor = msgCallbackFunctor;
}

void TcpServer::SetCloseCallback(CloseCallback closeCallbackFunctor)
{
    m_CloseCallbackFunctor = closeCallbackFunctor;
}

void TcpServer::SetListenAddr(const InetAddress &inetAddress)
{
    m_ListenAddr = inetAddress;
}

bool TcpServer::Start()
{
    if (m_IocpLoopPtr)
    {
        return true;
    }

    m_bQuit = false;	
    m_IocpLoopPtr = std::make_shared<IocpLoop>(*this);

    if (!m_IocpLoopPtr->Start(m_ListenAddr))
    {
        return false;
    }

    return true;
}

void TcpServer::Stop()
{
    if (!m_IocpLoopPtr)
    {
        return;
    }

    m_bQuit = true;

    m_NetEventQueue.Quit();
    m_IocpLoopPtr->Stop();

    m_IocpLoopPtr.reset();
}

void TcpServer::SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen)
{
    if (!pData || iLen <= 0 || !m_IocpLoopPtr)
    {
        return;
    }

    m_IocpLoopPtr->SendMessage(llClientHandle, pData, iLen);
}

void TcpServer::CloseClient(int64_t llClientHandle)
{
    if (!m_IocpLoopPtr)
    {
        return;
    }

    m_IocpLoopPtr->CloseClient(llClientHandle);
}

