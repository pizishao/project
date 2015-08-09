#pragma once

#include <thread>
#include <deque>

#include "IocpLoop.h"
#include "BlockingQueue.h"
#include "NetEventDispatcher.h"

typedef std::shared_ptr<IocpLoop> IocpLoopPtr;

class TcpServer
{	
public:
    TcpServer(NetEventDispatcher &dispatcher);
    ~TcpServer();

    friend class IocpLoop;

private:
    bool m_bQuit;

    InetAddress             m_listenAddr;

    IocpLoopPtr             m_iocpLoopPtr;
    BlockingQueue<NetEvent> m_netEventQueue;

    ConnectionCallback  m_connCallbackFunctor;
    MessageCallback	    m_msgCallbackFunctor;
    CloseCallback       m_closeCallbackFunctor;
    TimerCallback       m_timerCallbackFunctor;
	NetEventDispatcher& m_eventDispatcher;

private:
    void AddCloseNetEvent(int64_t llClientHandle);
    void AddMsgNetEvent(int64_t llClientHandle, PacketPtr &pktPtr);
    void AddConnNetEvent(int64_t llClientHandle);
    void AddTimerEvent();

public:
    void SetConnectionCallback(ConnectionCallback connCallbackFunctor);
    void SetMessageCallback(MessageCallback msgCallbackFunctor);
    void SetCloseCallback(CloseCallback closeCallbackFunctor);
    void SetTimerCallback(TimerCallback timerCallbackFunctor);

    void SetListenAddr(const InetAddress &inetAddress);
    bool Start();	
    void Stop();
    void SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen);
    void CloseClient(int64_t llClientHandle);
};
