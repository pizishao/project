#pragma once

#include <functional>
#include <stdint.h>
#include <memory>
#include <vector>
#include <set>
#include <mutex>
#include <deque>

#include <winsock2.h>

#include "Net.h"

typedef std::function<void(int64_t llClientHandle)> TimeOutCallback;

struct IdleConnection
{
    IdleConnection(TimeOutCallback onTimeOutFunctor, int64_t llClientHandle)
    {
        m_onTimeOutFunctor = onTimeOutFunctor;
        m_llClientHandle   = llClientHandle;
    }

    ~IdleConnection()
    {
        if (m_onTimeOutFunctor)
        {			
            //LOGWARN<<"client["<<m_llClientHandle<<"] time out";
            m_onTimeOutFunctor(m_llClientHandle);
        }
    }

    int64_t				m_llClientHandle;
    TimeOutCallback m_onTimeOutFunctor;
};

typedef std::shared_ptr<IdleConnection>	IdleConnPtr;
typedef std::weak_ptr<IdleConnection>	IdleConnWeakPtr;
typedef std::set<IdleConnPtr>			Bucket;
typedef std::vector <Bucket>				TimerWheelBuckets;

class TcpConnection
{
public:
    TcpConnection(SOCKET iFd);
    ~TcpConnection();

    SOCKET  GetFd();
    void    DestroySocket();

    void SetHandle(int64_t llClientHandle);
    int64_t  GetHandle();

    void AppendMessage(const void *data, int iLen);
    void Close();

    bool PostSend();
    void NotifySendHowMuchBytes(int iTranceCount);
    bool PostRead();
    bool NotifyReadHowMuchBytes(int iTranceCount);
    bool UnPack(std::vector<std::shared_ptr<Packet>> &vecPacket);
    bool IsUserClose();
    bool IsSendOver();	

    void SetIsRecving(bool value);
    bool IsRecving();
    void SetIsSending(bool value);
    bool IsSending();

    bool IsClosing();
    void SetIsClosing(bool value);

private:	
    SOCKET			 m_socket;
    int64_t			 m_llClientHandle;			

    std::mutex		  m_SendBufferlock;
    std::vector<char> m_vecSendBuffer;
    std::vector<char> m_vecTmpSendBuffer;
    bool		      m_bSendOver;
    std::vector<char> m_vecRecvBuffer;
    char              m_tmpRecvBuffer[POST_SIZE];

    bool m_isRecving;
    bool m_isSending;

    bool m_isClosing;

    OperateContext m_SendOpContext;
    OperateContext m_RecvOpContext;

    bool		 m_bCloseByUser;
};
