#pragma once

#include <WinSock2.h>
#include <thread>
#include <memory>
#include <unordered_map>

#include "Net.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "Accepter.h"
#include "Timer.h"

typedef std::shared_ptr<TcpConnection> ConnectionPtr;
typedef std::unordered_map<int64_t, TcpConnection *> MapConnection;
typedef std::vector<PostOperation>  UserOperationList;

class TcpServer;

class IocpLoop
{
public:
    IocpLoop(TcpServer &srv);
    ~IocpLoop();

public:
    bool Start(const InetAddress &inetAddress, int32_t iTimerMilliseconds);
    void Stop();

    void SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen);
    void CloseClient(int64_t llClientHandle);

private:

    bool AttachIocp(SOCKET sk, ULONG_PTR CompletionKey);
    bool AddConnection(SOCKET sk);

    TcpConnection* GetConnByHandle(int64_t llClientHandle);

    void Post(PostOperation::OpType iOpCode, int64_t llClientHandle = 0);

    void UserSendData(int64_t llClientHandle);
    void UserCloseClient(int64_t llClientHandle);
    void UserQuit();
    void ProcessWakeUp();
    void ProcessNotifySend(TcpConnection *pConn, int iTransferBytes);
    void ProcessNotifyRecv(TcpConnection *pConn, int iTransferBytes);

    int64_t	AllocateClientHandle();
    bool Init();
    void UnInit();
    void WaitLoop();

    void TryReleaseConn(TcpConnection *pConn);	
    void TryReleaseAllConn();
	bool CanQuit();

private:
    HANDLE      m_hIocp;
    bool        m_bClosing;

    InetAddress m_listenAddr;
    char        m_szRecvBuf[POST_SIZE];
    DWORD       m_recvLen;

    MapConnection	    m_mapConnection;
    std::mutex          m_mapConnLock;    

    UserOperationList	m_OpList;
    std::mutex			m_OpListLock;

    std::shared_ptr<std::thread> m_iocpWaitThreadPtr;

    Accepter  m_accepter;
    Timer     m_timer;
    TcpServer &m_TcpSrv;	
};
