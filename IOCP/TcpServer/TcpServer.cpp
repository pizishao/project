#include "TcpServer.h"

TcpServer::TcpServer(NetEventDispatcher &dispatcher) :
	m_eventDispatcher(dispatcher)
{
    m_bQuit = false;
	m_iTimerMilliseconds = 0;
}

TcpServer::~TcpServer()
{

}

void TcpServer::AddCloseNetEvent(int64_t llClientHandle)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Close;
    netEvent.m_llClientHandle=llClientHandle;
	netEvent.m_closeFunctor = m_closeCallbackFunctor;

    m_eventDispatcher.AddEvent(netEvent);

}

void TcpServer::AddMsgNetEvent(int64_t llClientHandle, PacketPtr &pktPtr)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Msg;
    netEvent.m_llClientHandle=llClientHandle;
    netEvent.m_pktPtr=pktPtr;
	netEvent.m_msgFunctor=m_msgCallbackFunctor;

    m_eventDispatcher.AddEvent(netEvent);
}

void TcpServer::AddConnNetEvent(int64_t llClientHandle)
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Connect;
    netEvent.m_llClientHandle=llClientHandle;
	netEvent.m_connFunctor=m_connCallbackFunctor;

    m_eventDispatcher.AddEvent(netEvent);
}

void TcpServer::AddTimerEvent()
{
    NetEvent netEvent;
    netEvent.m_eventType = NetEvent::en_Timer;
    netEvent.m_timerCallbackFunctor = m_timerCallbackFunctor;

    m_eventDispatcher.AddEvent(netEvent);
}

void TcpServer::SetConnectionCallback(ConnectionCallback connCallbackFunctor)
{
    m_connCallbackFunctor = connCallbackFunctor;
}

void TcpServer::SetMessageCallback(MessageCallback msgCallbackFunctor)
{
    m_msgCallbackFunctor = msgCallbackFunctor;
}

void TcpServer::SetCloseCallback(CloseCallback closeCallbackFunctor)
{
    m_closeCallbackFunctor = closeCallbackFunctor;
}

void TcpServer::SetTimer(int32_t iTimerMilliseconds, TimerCallback timerCallbackFunctor)
{
	m_iTimerMilliseconds=iTimerMilliseconds;
    m_timerCallbackFunctor = timerCallbackFunctor;
}

void TcpServer::SetListenAddr(const InetAddress &inetAddress)
{
    m_listenAddr = inetAddress;
}

bool TcpServer::Start()
{
    if (m_iocpLoopPtr)
    {
        return true;
    }

    m_bQuit = false;	
    m_iocpLoopPtr = std::make_shared<IocpLoop>(*this);

    if (!m_iocpLoopPtr->Start(m_listenAddr, m_iTimerMilliseconds))
    {
        return false;
    }

    return true;
}

void TcpServer::Stop()
{
    if (!m_iocpLoopPtr)
    {
        return;
    }

    m_bQuit = true;

    m_iocpLoopPtr->Stop();

    m_iocpLoopPtr.reset();
}

void TcpServer::SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen)
{
    if (!pData || iLen <= 0 || !m_iocpLoopPtr)
    {
        return;
    }

    m_iocpLoopPtr->SendMessage(llClientHandle, pData, iLen);
}

void TcpServer::CloseClient(int64_t llClientHandle)
{
    if (!m_iocpLoopPtr)
    {
        return;
    }

    m_iocpLoopPtr->CloseClient(llClientHandle);
}

