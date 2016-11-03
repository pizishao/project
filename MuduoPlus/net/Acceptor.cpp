#include "Acceptor.h"
#include "SocketOps.h"
#include "base/Logger.h"
#include "EventLoop.h"

namespace MuduoPlus
{

    Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    {
        loop_           = loop;
        listenAddr_     = listenAddr;
        isReuseport_    = reuseport;
        listenFd_       = -1;
        listenning_     = false;
    }

    Acceptor::~Acceptor()
    {
        acceptChannelPtr_->disableAll();
        acceptChannelPtr_->remove();
        SocketOps::closeSocket(listenFd_);
    }

    void Acceptor::listen()
    {
        socket_t fd = SocketOps::createSocket();

        if(fd < 0)
        {
            LOG_PRINT(LogType_Fatal, "create socket failed:%s %s:%d",
                      GetLastErrorText().c_str(), __FUNCTION__, __LINE__);
            return;
        }

        if(!SocketOps::bindSocket(fd, &listenAddr_.getSockAddr()))
        {
            LOG_PRINT(LogType_Fatal, "bind socket failed:%s %s:%d",
                      GetLastErrorText().c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        if(!SocketOps::listen(fd))
        {
            LOG_PRINT(LogType_Fatal, "listen socket failed:%s %s:%d",
                      GetLastErrorText().c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        if(!SocketOps::setSocketNoneBlocking(fd))
        {
            LOG_PRINT(LogType_Fatal, "enable socket noneBlocking failed:%s %s:%d",
                      GetLastErrorText().c_str(), __FUNCTION__, __LINE__);
            goto err;
        }

        LOG_PRINT(LogType_Info, "Acceptor listen at ip:%s port:%u", listenAddr_.ip().c_str(),
                  listenAddr_.port());

        listenFd_ = fd;
        acceptChannelPtr_ = std::make_shared<Channel>(loop_, listenFd_);
        acceptChannelPtr_->setReadCallback(std::bind(&Acceptor::handleRead, this));
        acceptChannelPtr_->setOwner(shared_from_this());
        acceptChannelPtr_->enableReading();
        listenning_ = true;

        return;

err:
        SocketOps::closeSocket(fd);
        return;
    }

    void Acceptor::handleRead()
    {
        loop_->assertInLoopThread();
        int errorCode = 0;

        while(true)
        {
            InetAddress peerAddr;
            sockaddr addr = {0};

            socket_t newFd = SocketOps::accept(listenFd_, &addr);

            if(newFd < 0)
            {
                errorCode = GetLastErrorCode();
                break;
            }

            SocketOps::setSocketNoneBlocking(newFd);
            peerAddr.setSockAddr(addr);

            if(newConnCallBack_)
            {
                newConnCallBack_(newFd, peerAddr);
            }
            else
            {
                SocketOps::closeSocket(newFd);
            }
        }

        if(!ERR_ACCEPT_RETRIABLE(errorCode))
        {
            LOG_PRINT(LogType_Error, "accept socket failed:%s %s:%d",
                      GetErrorText(errorCode).c_str(), __FUNCTION__, __LINE__);
        }
    }
}