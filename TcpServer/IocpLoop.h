#pragma once

#include <WinSock2.h>
#include <thread>
#include <memory>
#include <unordered_map>

#include "Net.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "Accepter.h"

typedef std::shared_ptr<TcpConnection> ConnectionPtr;

class TcpServer;

class IocpLoop
{
public:
    IocpLoop(TcpServer &srv);
    ~IocpLoop();

public:
    bool Start(const InetAddress &inetAddress);
    void Stop();

    void SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen);
    void CloseClient(int64_t llClientHandle);

private:

    bool AttachIocp(SOCKET sk, ULONG_PTR CompletionKey);
    bool AddConnection(SOCKET sk);

    TcpConnection* GetConnByHandle(int64_t llClientHandle);

    void PostUserOperation(int64_t llClientHandle, UserOperation::OpType iOpCode);

    void UserSendData(int64_t llClientHandle);
    void UserCloseClient(int64_t llClientHandle);
    void UserQuit();
    bool ProcessWakeUp();
    void ProcessNotifySend(TcpConnection *pConn, int iTranceCount);
    void ProcessNotifyRecv(TcpConnection *pConn, int iTranceCount);

    int64_t	AllocateClientHandle();
    bool Init();
    void UnInit();
    void WaitLoop();

    //void ReleaseConn(int64_t llClientHandle);
    void TryReleaseConn(TcpConnection *pConn);	
    void ReleaseAllConn();

private:
    HANDLE m_hIocp;
    bool   m_bQuit;

    InetAddress m_listenAddr;
    char        m_szRecvBuf[POST_SIZE];
    DWORD       m_recvLen;

    std::unordered_map<int64_t, TcpConnection *>	m_mapConnection;
    std::mutex						                m_mapConnLock;

    std::vector<UserOperation>	m_vecOp;
    std::mutex					m_vecOpLock;

    std::shared_ptr<std::thread> m_iocpWaitThreadPtr;

    Accepter  m_accepter;
    TcpServer &m_TcpSrv;	
};
