#include "IocpLoop.h"
#include "Sock.h"
#include "TcpServer.h"

#include <Mswsock.h>
#include <iostream>
#include <assert.h>
#include <exception>

using namespace std;

IocpLoop::IocpLoop(TcpServer &srv)
    : m_TcpSrv(srv)
{
    m_hIocp = nullptr;
    m_bQuit = false;
    memset(m_szRecvBuf, 0, POST_SIZE);
    m_recvLen = 0;
}

IocpLoop::~IocpLoop()
{
}

bool IocpLoop::AttachIocp(SOCKET sk, ULONG_PTR CompletionKey)
{
    if (CreateIoCompletionPort((HANDLE)sk, m_hIocp, CompletionKey, 0) == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool IocpLoop::AddConnection(SOCKET sk)
{
    TcpConnection *pConn = new TcpConnection(sk);
    pConn->SetHandle(AllocateClientHandle());

    if (!AttachIocp(sk, (ULONG_PTR)pConn))
    {
        //assert(false);
        delete pConn;
        return false;
    }

    if (!pConn->PostRead())
    {
        //assert(false);
        delete pConn;
        return false;
    }    

    m_mapConnLock.lock();
    m_mapConnection.insert(std::make_pair(pConn->GetHandle(), pConn));	
    m_mapConnLock.unlock();

    m_TcpSrv.AddConnNetEvent(pConn->GetHandle());

    return true;
}

TcpConnection* IocpLoop::GetConnByHandle(int64_t llClientHandle)
{
    auto found = m_mapConnection.find(llClientHandle);
    if (found != m_mapConnection.end())
    {
        return found->second;
    }

    return nullptr;
}

void IocpLoop::PostUserOperation(int64_t llClientHandle, UserOperation::OpType iOpCode)
{
    UserOperation op;
    op.llClientHandle = llClientHandle;
    op.type = iOpCode;

    std::lock_guard<std::mutex> lockGuard(m_vecOpLock);
    m_vecOp.push_back(op);
    PostQueuedCompletionStatus(m_hIocp, 0, OP_Post, nullptr);
}

void IocpLoop::UserSendData(int64_t llClientHandle)
{
    auto pConn = GetConnByHandle(llClientHandle);

    if (pConn)
    {			
        if (!pConn->IsSending() && !pConn->IsClosing())
        {
            if (!pConn->PostSend()) // 发送数据
            {
                TryReleaseConn(pConn);
            }
        }                
    }
}

void IocpLoop::UserCloseClient(int64_t llClientHandle)
{
    auto pConn = GetConnByHandle(llClientHandle);

    if (pConn)
    {				
        if (pConn->IsSendOver())
        {
            TryReleaseConn(pConn);
        }
    }
}

void IocpLoop::UserQuit()
{
    ReleaseAllConn();
    m_accepter.Close();

    m_bQuit=true;
}

bool IocpLoop::ProcessWakeUp()
{
    std::vector<UserOperation> vecOp;

    m_vecOpLock.lock();
    vecOp.swap(m_vecOp);
    m_vecOpLock.unlock();

    for (auto &pos:vecOp)
    {
        if (pos.type == UserOperation::en_SendData)
        {
            UserSendData(pos.llClientHandle);
        }
        else if (pos.type == UserOperation::en_Close)
        {
            UserCloseClient(pos.llClientHandle);
        }
        else if (pos.type == UserOperation::en_Quit)
        {
            UserQuit();
        }
    }

    return true;
}

void IocpLoop::ProcessNotifySend(TcpConnection *pConn, int iTranceCount)
{
    pConn->SetIsSending(false);

    if (iTranceCount == 0)
    {
        TryReleaseConn(pConn);
        return;
    }

    pConn->NotifySendHowMuchBytes(iTranceCount);
    if (pConn->IsSendOver() && pConn->IsUserClose())
    {
        TryReleaseConn(pConn);
    }
    else
    {
        if (!pConn->IsClosing())
        {
            if (!pConn->PostSend())
            {
                TryReleaseConn(pConn);
            }                
        }
        else
        {
            TryReleaseConn(pConn);
        }
    }
}

void IocpLoop::ProcessNotifyRecv(TcpConnection *pConn, int iTranceCount)
{
    pConn->SetIsRecving(false);

    if (iTranceCount == 0)
    {
        TryReleaseConn(pConn);
        return;
    }

    if (!pConn->NotifyReadHowMuchBytes(iTranceCount))
    {
        TryReleaseConn(pConn);
        return;
    }

    std::vector<std::shared_ptr<Packet>> vecPacket;
    if (!pConn->UnPack(vecPacket)) // 尝试解包
    {
        TryReleaseConn(pConn);
        return;
    }
    else
    {
        for (auto &pos:vecPacket)
        {
            m_TcpSrv.AddMsgNetEvent(pConn->GetHandle(), pos);
        }
    }

    if (!pConn->IsClosing())
    {
        if (!pConn->PostRead())
        {
            TryReleaseConn(pConn);
        }            
    }
    else
    {
        TryReleaseConn(pConn);
    }
}

int64_t	IocpLoop::AllocateClientHandle()
{
    static int64_t llClientHandle = 1;

    return llClientHandle++;
}

bool IocpLoop::Init()
{
    m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    
    if (!m_hIocp)
    {
        cout<<"CreateIoCompletionPort error "<<GetLastError()<<endl;
        return false;
    }

    m_accepter.SetAttachIocpFunctor(std::bind(&IocpLoop::AttachIocp, this, 
        std::placeholders::_1, std::placeholders::_2));
    m_accepter.SetAddConnFunctor(std::bind(&IocpLoop::AddConnection, this,
        std::placeholders::_1));

    if (!m_accepter.Listen(m_listenAddr))
    {
        return false;
    }    

    return true;
}

void IocpLoop::UnInit()
{
    SAFE_CLOSE_HANDLE(m_hIocp);
}

bool IocpLoop::Start(const InetAddress &inetAddress)
{
    m_listenAddr=inetAddress;

    if (!Init())
    {
        return false;
    }

    m_iocpWaitThreadPtr = make_shared<std::thread>(&IocpLoop::WaitLoop, this);

    return true;
}

void IocpLoop::Stop()
{
    m_iocpWaitThreadPtr->join();	
    m_iocpWaitThreadPtr.reset();

    UnInit();
}

void IocpLoop::SendMessage(int64_t llClientHandle, const void *pData ,int32_t iLen)
{
    lock_guard<mutex> lockGuard(m_mapConnLock);
    auto pConn = GetConnByHandle(llClientHandle);
    if (pConn)
    {		
        if (pConn->IsUserClose()) // 上层已经关闭发送数据则返回
        {
            //LOGWARN<<"client already closed but still send data";
            return;
        }

        pConn->AppendMessage(pData, iLen);

        if (!pConn->IsSending())
        {
            PostUserOperation(llClientHandle, UserOperation::en_SendData);
        }        
    }
    else
    {
        //LOGWARN<<"can't find client["<<llClientHandle<<"]";
    }
}

void IocpLoop::CloseClient(int64_t llClientHandle)
{
    std::lock_guard<std::mutex> lockGuard(m_mapConnLock);
    auto pConn = GetConnByHandle(llClientHandle);
    if (pConn)
    {
        pConn->Close();	

        PostUserOperation(llClientHandle, UserOperation::en_Close);
    }
}

void IocpLoop::TryReleaseConn(TcpConnection *pConn)
{
    pConn->SetIsClosing(true);

    if (pConn->IsRecving() || pConn->IsSending())
    {
        pConn->DestroySocket();
        return;
    }    
    
    if (!pConn->IsUserClose())
    {
        m_TcpSrv.AddCloseNetEvent(pConn->GetHandle());
    }

    std::lock_guard<std::mutex> lockGuard(m_mapConnLock);
    m_mapConnection.erase(pConn->GetHandle());

    delete pConn;
}

void IocpLoop::ReleaseAllConn()
{
    std::lock_guard<std::mutex> lockGuard(m_mapConnLock);

    for (auto &pos:m_mapConnection)
    {
        delete pos.second;
    }

    m_mapConnection.clear();
}

void IocpLoop::WaitLoop()
{
    while (!m_bQuit)
    {
        DWORD dwTranceCount = 0;
        ULONG_PTR uComKey = NULL;
        LPOVERLAPPED pOverlapped = NULL;

        BOOL bRet = GetQueuedCompletionStatus(m_hIocp, &dwTranceCount, &uComKey, &pOverlapped, INFINITE);

        if (!bRet)
        {
            if (!pOverlapped)
            {
                assert(false);
            }
            else
            {
                IOCP_OPERATION ioType = ((OperateContext*)pOverlapped)->m_IoType;
                TcpConnection *pConn = (TcpConnection *)uComKey;

                if (ioType == OP_Send)
                {
                    pConn->SetIsSending(false);
                    TryReleaseConn(pConn);
                }
                else if (ioType == OP_Recv)
                {
                    pConn->SetIsRecving(false);
                    TryReleaseConn(pConn);
                }
                else
                {                        
                    m_accepter.NotifyAccept(false);
                }
            }

            continue;
        }

        if (uComKey == OP_Post)
        {
            ProcessWakeUp();
            continue;
        }

        assert(pOverlapped);

        //! 处理来自网络的通知
        IOCP_OPERATION ioType = ((OperateContext*)pOverlapped)->m_IoType;
        switch (ioType)
        {
        case OP_Accept:
            {
                m_accepter.NotifyAccept(true);
            }
            break;

        case OP_Send:
            {
                TcpConnection *pConn = (TcpConnection *)uComKey;
                ProcessNotifySend(pConn, dwTranceCount);
            }
            break;

        case OP_Recv:
            {
                TcpConnection *pConn = (TcpConnection *)uComKey;
                ProcessNotifyRecv(pConn, dwTranceCount);
            }            
            break;
        }
    }    
}
