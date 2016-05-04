#include <assert.h>
#include <algorithm>

#include "Connector.h"
#include "EventLoop.h"
#include "Channel.h"
#include "base/Logger.h"

namespace MuduoPlus
{
    const int Connector::kMaxRetryDelayMs;

    Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
        : loop_(loop),
        serverAddr_(serverAddr),
        connect_(false),
        state_(kDisconnected),
        retryDelayMs_(kInitRetryDelayMs)
    {
        //LOG_DEBUG << "ctor[" << this << "]";
    }

    Connector::~Connector()
    {
        LOG_PRINT(LogType_Debug, "dtor[%p]", this);
        assert(!channelPtr_);
    }

    void Connector::start()
    {
        connect_ = true;
        loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
    }

    void Connector::startInLoop()
    {
        loop_->assertInLoopThread();
        assert(state_ == kDisconnected);
        if (connect_)
        {
            connect();
        }
        else
        {
            LOG_PRINT(LogType_Debug, "do not connect");
        }
    }

    void Connector::stop()
    {
        connect_ = false;
        loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
        // FIXME: cancel timer
    }

    void Connector::stopInLoop()
    {
        loop_->assertInLoopThread();
        if (state_ == kConnecting)
        {
            setState(kDisconnected);
            int sockfd = removeAndResetChannel();
            retry(sockfd);
        }
    }

    void Connector::connect()
    {
        int sockFd = SocketOps::createSocket();
        SocketOps::setSocketNoneBlocking(sockFd);

        int ret = SocketOps::connect(sockFd, &serverAddr_.getSockAddr());
        if (ret == 0) // connect success immediately
        {
            setState(kConnected);

            if (connect_)
            {
                newConnectionCallback_(sockFd);
            }
            else
            {
                SocketOps::closeSocket(sockFd);
            }
        }
        else
        {
            int errorCode = GetLastErrorCode();

            if (ERR_CONNECT_RETRIABLE(errorCode))
            {
                connecting(sockFd);
            }
            else
            {
                retry(sockFd);
            }
        }
    }

    void Connector::restart()
    {
        loop_->assertInLoopThread();
        setState(kDisconnected);
        retryDelayMs_ = kInitRetryDelayMs;
        connect_ = true;
        startInLoop();
    }

    void Connector::connecting(int sockfd)
    {
        setState(kConnecting);
        assert(!channelPtr_);
        channelPtr_.reset(new Channel(loop_, sockfd));
        channelPtr_->setWriteCallback(
            std::bind(&Connector::handleWrite, this));
        channelPtr_->setErrorCallback(
            std::bind(&Connector::handleError, this));

        // channelPtr_->tie(shared_from_this()); is not working,
        // as channelPtr_ is not managed by shared_ptr
        channelPtr_->enableWriting();
        channelPtr_->enableErroring();
    }

    int Connector::removeAndResetChannel()
    {
        channelPtr_->disableAll();
        channelPtr_->remove();
        int sockfd = channelPtr_->fd();
        // Can't reset channel_ here, because we are inside Channel::handleEvent
        loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
        return sockfd;
    }

    void Connector::resetChannel()
    {
        channelPtr_.reset();
    }

    void Connector::handleWrite()
    {
        LOG_PRINT(LogType_Debug, "Connector::handleWrite %u", state_);

        if (state_ == kConnecting)
        {
            int sockfd = removeAndResetChannel();
            int err = SocketOps::getSocketError(sockfd);

            if (err)           
            {
                LOG_PRINT(LogType_Warn, "Connector::handleWrite - SO_ERROR = %u %s",
                    err, GetErrorText(err).c_str());
                retry(sockfd);
            }
            else
            {
                setState(kConnected);
                if (connect_)
                {
                    newConnectionCallback_(sockfd);
                }
                else
                {
                    SocketOps::closeSocket(sockfd);
                }
            }
        }
        else
        {
            // what happened?
            assert(state_ == kDisconnected);
        }
    }

    void Connector::handleError()
    {
        LOG_PRINT(LogType_Error, "Connector::handleError state=%u", state_);
        if (state_ == kConnecting)
        {
            int sockfd = removeAndResetChannel();
            int err = SocketOps::getSocketError(sockfd);
            LOG_PRINT(LogType_Debug, "SO_ERROR = %u %s", err, GetErrorText(err).c_str());
            retry(sockfd);
        }
    }

    void Connector::retry(int sockfd)
    {
        SocketOps::closeSocket(sockfd);
        setState(kDisconnected);

        if (connect_)
        {
            LOG_PRINT(LogType_Info, "Connector::retry - Retry connecting to %s in %d milliseconds. ", 
                serverAddr_.toIpPort().c_str(), retryDelayMs_);

            loop_->runAfter(retryDelayMs_ / 1000.0, 
                std::bind(&Connector::startInLoop, shared_from_this()));

            retryDelayMs_ = (std::min)(retryDelayMs_ * 2, kMaxRetryDelayMs);
        }
        else
        {
            LOG_PRINT(LogType_Debug, "do not connect");
        }
    }
}