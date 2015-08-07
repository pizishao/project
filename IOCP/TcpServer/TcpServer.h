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

    InetAddress m_ListenAddr;

    IocpLoopPtr m_IocpLoopPtr;
    BlockingQueue<NetEvent> m_NetEventQueue;

    ConnectionCallback m_ConnCallbackFunctor;
    MessageCallback	m_MsgCallbackFunctor;
    CloseCallback m_CloseCallbackFunctor;
	NetEventDispatcher &m_EventDispatcher;

private:
    void AddCloseNetEvent(int64_t llClientHandle);
    void AddMsgNetEvent(int64_t llClientHandle, std::shared_ptr<Packet> &pkt);
    void AddConnNetEvent(int64_t llClientHandle);    

public:
    void SetConnectionCallback(ConnectionCallback connCallbackFunctor);
    void SetMessageCallback(MessageCallback msgCallbackFunctor);
    void SetCloseCallback(CloseCallback closeCallbackFunctor);

    void SetListenAddr(const InetAddress &inetAddress);
    bool Start();	
    void Stop();
    void SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen);
    void CloseClient(int64_t llClientHandle);
};
