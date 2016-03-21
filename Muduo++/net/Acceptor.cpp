#include "Acceptor.h"
#include "SocketOps.h"
#include "base/Logger.h"

namespace MuduoPlus
{

    Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    {
        m_pEventLoop    = loop;
        m_ListenAddr    = listenAddr;
        m_bReuseport    = reuseport;
        m_ListenFd      = -1;
    }

    Acceptor::~Acceptor()
    {
        m_AcceptChannelPtr->disableAll();
        m_AcceptChannelPtr->remove();
        SocketOps::closeSocket(m_ListenFd);
    }

    bool Acceptor::Listen()
    {
        socket_t fd = SocketOps::createSocket();

        if (fd == -1)
        {           
            LOG_PRINT(LogType_Error, "create socket failed:%s %s:%d", 
                GetErrorText(GetErrorCode()).c_str(), __FUNCTION__, __LINE__);
            return false;
        }

        if (!SocketOps::setSocketNoneBlocking(fd))
        {
            LOG_PRINT(LogType_Error, "enable socket noneBlocking failed:%s %s:%d", 
                GetErrorText(GetErrorCode()).c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        if (!SocketOps::bindSocket(fd, (sockaddr *)&m_ListenAddr.getSockAddrIn()))
        {
            LOG_PRINT(LogType_Error, "bind socket failed:%s %s:%d", 
                GetErrorText(GetErrorCode()).c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        if (!SocketOps::listen(fd))
        {
            LOG_PRINT(LogType_Error, "listen socket failed:%s %s:%d", 
                GetErrorText(GetErrorCode()).c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        LOG_PRINT(LogType_Info, "listen at ip:%s port:%u", m_ListenAddr.ip().c_str(), m_ListenAddr.port());

        m_ListenFd = fd;
        m_AcceptChannelPtr = std::make_shared<Channel>(m_pEventLoop, m_ListenFd);
        m_AcceptChannelPtr->setReadCallback(std::bind(&Acceptor::HandleRead, this));
        m_AcceptChannelPtr->setOwner(shared_from_this());
        m_AcceptChannelPtr->enableReading();

        return true;

    err:
        SocketOps::closeSocket(fd);
        return false;
    }

    void Acceptor::HandleRead()
    {
        //m_pEventLoop->assertInLoopThread();        

        while (true)
        {
            InetAddress peerAddr;
            sockaddr addr = {0};

            socket_t newFd = SocketOps::accept(m_ListenFd, &addr);
            if (newFd < 0)
            {
                break;
            }

            peerAddr.setSockAddrIn(*(sockaddr_in *)&addr);
            if (m_NewConnCallBack)
            {               
                m_NewConnCallBack(newFd, peerAddr);
            }
            else
            {
                SocketOps::closeSocket(newFd);
            }
        }

        int errorCode = GetErrorCode();
        if (!ERR_ACCEPT_RETRIABLE(errorCode)) 
        {
            LOG_PRINT(LogType_Error, "accept socket failed:%s %s:%d",
                GetErrorText(errorCode).c_str(), __FUNCTION__, __LINE__);
        }
    }
}