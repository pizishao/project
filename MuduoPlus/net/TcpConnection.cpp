#include "base/Logger.h"
#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    void defaultConnectionCallback(const TcpConnectionPtr& conn)
    {
        LOG_PRINT(LogType_Info, "%s -> %s is %s", conn->localAddress().toIpPort().c_str(),
            conn->peerAddress().toIpPort().c_str(), conn->connected() ? "UP" : "DOWN");
    }

    void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp receiveTime)
    {
        buf->retrieveAll();
    }

    TcpConnection::TcpConnection(EventLoop* loop,
        const std::string& nameArg,
        int sockfd,
        const InetAddress& localAddr,
        const InetAddress& peerAddr)
        : loop_(loop),
        name_(nameArg),
        state_(kConnecting),
        fd_(sockfd),
        sockErrorOccurred_(false),
        userClosed_(false),
        channel_(new Channel(loop, fd_)),
        localAddr_(localAddr),
        peerAddr_(peerAddr),
        highWaterMark_(64 * 1024 * 1024),
        reading_(true)
    {
        channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
        channel_->setWriteCallback(
            std::bind(&TcpConnection::handleWrite, this));
        channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError, this));
        channel_->setFinishCallback(std::bind(&TcpConnection::handleFinish, this));

        SocketOps::setKeepAlive(fd_, true);
    }

    TcpConnection::~TcpConnection()
    {
        LOG_PRINT(LogType_Info, "TcpConnection::dtor[%s] at %p fd=%d", name_.c_str(), this,
            channel_->fd());    
        assert(state_ == kDisconnected);

        SocketOps::closeSocket(fd_);
    }

    void TcpConnection::send(const void* data, int len)
    {
        if (len <= 0)
        {
            return;
        }

        if (state_ == kConnected)
        {
            if (loop_->isInLoopThread())
            {
                sendInLoop(data, len);
            }
            else
            {
                auto vecData = std::make_shared<vector_char>((char *)data, (char *)data + len);
                auto selfPtr = shared_from_this();

                loop_->runInLoop([=]()
                {
                    selfPtr->sendInLoop(vecData);
                });
            }
        }
    }

    void TcpConnection::sendInLoop(std::shared_ptr<vector_char> vecData)
    {        
        loop_->assertInLoopThread();

        if (state_ == kDisconnected)
        {
            LOG_PRINT(LogType_Warn, "disconnected, give up writing");
            return;
        }

        sendInLoop(vecData->data(), vecData->size());
    }

    void TcpConnection::sendInLoop(const void* data, int len)
    {        
        loop_->assertInLoopThread();

        if (state_ == kDisconnected)
        {
            LOG_PRINT(LogType_Warn, "disconnected, give up writing");
            return;
        }

        int sendCount = 0;
        int remainCount = len;

        // if no thing in output queue, try writing directly
        if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
        {
            sendCount = SocketOps::send(channel_->fd(), data, len);
            if (sendCount >= 0)
            {
                remainCount = len - sendCount;
                if (remainCount == 0 && writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
            }
            else // sendCount < 0
            {
                if (!ERR_RW_RETRIABLE(GetLastErrorCode()))
                {
                    LOG_PRINT(LogType_Error, "fd[%d] send failed:%s", 
                        fd_, GetLastErrorText().c_str());
                    sockErrorOccurred_ = true;
                }
            }
        }

        assert(remainCount <= len);
        if (!sockErrorOccurred_ && remainCount > 0)
        {
            size_t oldLen = outputBuffer_.readableBytes();
            if (oldLen + remainCount >= highWaterMark_
                && oldLen < highWaterMark_
                && highWaterMarkCallback_)
            {
                loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remainCount));
            }

            outputBuffer_.append(static_cast<const char*>(data)+sendCount, remainCount);
            if (!channel_->isWriting())
            {
                channel_->enableWriting();
            }
        }

        if (!sockErrorOccurred_)
        {
            if (remainCount == 0)
            {
                LOG_PRINT(LogType_Debug, "send over");

                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
    }

    void TcpConnection::gracefulClose()
    {
        if (state_ == kConnected)
        {
            setState(kDisconnecting);
            auto selfPtr = shared_from_this();

            loop_->runInLoop([=]()
            {
                selfPtr->shutdownInLoop();
            });
        }
    }

    void TcpConnection::shutdownInLoop()
    {
        if (state_ == kConnected && channel_->isWriting())
        {
            return;
        }

        loop_->assertInLoopThread();
        assert(!channel_->isWriting());
        userClosed_ = true;
        //if (!channel_->isWriting())
        {
            //SocketOps::ShutdownWrite(fd_);
            releaseConnection();
        }        
    }

    void TcpConnection::forceClose()
    {
        if (state_ == kConnected || state_ == kDisconnecting)
        {
            setState(kDisconnecting);
            loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
        }
    }

    void TcpConnection::forceCloseWithDelay(double seconds)
    {
        if (state_ == kConnected || state_ == kDisconnecting)
        {
            setState(kDisconnecting);
            auto selfPtr = shared_from_this();
            loop_->runAfter(seconds, [=]()
            {
                selfPtr->forceClose();
            });
        }
    }

    void TcpConnection::forceCloseInLoop()
    {
        loop_->assertInLoopThread();
        userClosed_ = true;  

        releaseConnection();
    }

    void TcpConnection::releaseConnection()
    {
        if (state_ == kConnected || state_ == kDisconnecting)
        {
            setState(kDisconnected);
            channel_->disableAll();  
            channel_->remove();

            if (closeCallback_)
            {
                closeCallback_(shared_from_this());
                closeCallback_ = nullptr;
            }            
        }
    }

    const char* TcpConnection::stateToString() const
    {
        switch (state_)
        {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
        }
    }

    void TcpConnection::setTcpNoDelay(bool on)
    {
        SocketOps::setTcpNoDelay(fd_, on);
    }

    void TcpConnection::startRead()
    {
        auto selfPtr = shared_from_this();

        loop_->runInLoop([=]()
        {
            selfPtr->startReadInLoop();
        });
    }

    void TcpConnection::startReadInLoop()
    {
        loop_->assertInLoopThread();
        if (!reading_ || !channel_->isReading())
        {
            channel_->enableReading();
            reading_ = true;
        }
    }

    void TcpConnection::stopRead()
    {
        auto selfPtr = shared_from_this();

        loop_->runInLoop([=]()
        {
            selfPtr->stopReadInLoop();
        });
    }

    void TcpConnection::stopReadInLoop()
    {
        loop_->assertInLoopThread();
        if (reading_ || channel_->isReading())
        {
            channel_->disableReading();
            reading_ = false;
        }
    }

    void TcpConnection::connectEstablished()
    {
        auto selfPtr = shared_from_this();
        assert(state_ == kConnecting);
        setState(kConnected);
        loop_->assertInLoopThread();
        channel_->setOwner(selfPtr);
        channel_->enableReading();
        channel_->enableErroring();

        connectionCallback_(selfPtr);
    }

    void TcpConnection::connectDestroyed()
    {
        loop_->assertInLoopThread();
        /*if (state_ == kConnected)
        {
            setState(kDisconnected);
            channel_->disableAll();            
        }*/

        assert(state_ == kDisconnected);

        if (!userClosed_)
        {
            connectionCallback_(shared_from_this());
        }        
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        loop_->assertInLoopThread();
        bool ret = inputBuffer_.readFd(channel_->fd());
        if (ret)
        {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
        else
        {
            sockErrorOccurred_ = true;
        }
    }

    void TcpConnection::handleWrite()
    {
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
            size_t n = SocketOps::send(channel_->fd(),
                outputBuffer_.peek(),
                outputBuffer_.readableBytes());
            if (n > 0)
            {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                    channel_->disableWriting();
                    if (writeCompleteCallback_)
                    {
                        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                    }
                    
                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop();
                    }

                    LOG_PRINT(LogType_Error, "TcpConnection::send all data");
                }
            }
            else
            {
                if (!ERR_RW_RETRIABLE(GetLastErrorCode()))
                {
                    sockErrorOccurred_ = true;
                    LOG_PRINT(LogType_Error, "TcpConnection::handleWrite");
                }
            }
        }
        else
        {
            LOG_PRINT(LogType_Info, "Connection fd = %d is down, no more writing", channel_->fd());
        }
    }

    void TcpConnection::handleError()
    {
        loop_->assertInLoopThread();
        assert(state_ == kConnected || state_ == kDisconnecting);
                
        sockErrorOccurred_ = true;
    }

    void TcpConnection::handleFinish()
    {
        /*setState(kDisconnected);
        channel_->disableAll();*/        

        if (sockErrorOccurred_ /*&& closeCallback_*/)
        {
            releaseConnection();
            /*closeCallback_(shared_from_this());
            closeCallback_ = nullptr;*/
        }        
    }
}