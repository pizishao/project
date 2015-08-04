#include "TcpServer.h"

TcpServer::TcpServer(NetEventDispatcher &dispatcher) :
	m_EventDispatcher(dispatcher)
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
	netEvent.m_closeFunctor = m_CloseCallbackFunctor;

    m_EventDispatcher.AddEvent(netEvent);

}

void TcpServer::AddMsgNetEvent(int64_t llClientHandle, std::shared_ptr<Packet> &pkt)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Msg;
    netEvent.m_llClientHandle=llClientHandle;
    netEvent.m_pkt=pkt;
	netEvent.m_msgFunctor=m_MsgCallbackFunctor;

    m_EventDispatcher.AddEvent(netEvent);
}

void TcpServer::AddConnNetEvent(int64_t llClientHandle)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Connect;
    netEvent.m_llClientHandle=llClientHandle;
	netEvent.m_connFunctor=m_ConnCallbackFunctor;

    m_EventDispatcher.AddEvent(netEvent);
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

