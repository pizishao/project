#include "base/Logger.h"
#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    TcpConnection::TcpConnection(EventLoop* loop,
        const std::string& nameArg,
        int sockfd,
        const InetAddress& localAddr,
        const InetAddress& peerAddr)
        : loop_(loop),
        name_(nameArg),
        fd_(sockfd),
        sockErrorOccurred(false),
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
        channel_->setCloseCallback(
            std::bind(&TcpConnection::handleClose, this));
        channel_->setFinishCallback(
            std::bind(&TcpConnection::handleFinish, this));

        SocketOps::SetKeepAlive(fd_, true);
    }

    TcpConnection::~TcpConnection()
    {
        LOG_PRINT(LogType_Info, "TcpConnection::dtor[%s] at %p fd=%d", name_.c_str(), this,
            channel_->fd());        
    }

    void TcpConnection::send(const void* data, int len)
    {
        if (len <= 0)
        {
            return;
        }

        if (loop_->isInLoopThread())
        {
            sendInLoop(data, len);
        }
        else
        {
            auto vecData = std::make_shared<vector_char>();
            auto selfPtr = shared_from_this();

            loop_->runInLoop([=]()
            {
                selfPtr->sendInLoop(vecData);
            });
        }
    }

    void TcpConnection::sendInLoop(std::shared_ptr<vector_char> vecData)
    {
        loop_->assertInLoopThread();
        sendInLoop(vecData->data(), vecData->size());
    }

    void TcpConnection::sendInLoop(const void* data, size_t len)
    {
        loop_->assertInLoopThread();
        size_t sendCount = 0;
        size_t left = len;
        bool faultError = false;

        // if no thing in output queue, try writing directly
        if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
        {
            sendCount = SocketOps::Send(channel_->fd(), data, len);
            if (sendCount >= 0)
            {
                left = len - sendCount;
                if (left == 0 && writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
            }
            else // sendCount < 0
            {
                if (!ERR_RW_RETRIABLE(GetErrorCode()))
                {
                    sockErrorOccurred = true;
                    faultError = true;                    
                }
            }
        }

        assert(left <= len);
        if (!faultError && left > 0)
        {
            size_t oldLen = outputBuffer_.readableBytes();
            if (oldLen + left >= highWaterMark_
                && oldLen < highWaterMark_
                && highWaterMarkCallback_)
            {
                loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + left));
            }

            outputBuffer_.append(static_cast<const char*>(data)+sendCount, left);
            if (!channel_->isWriting())
            {
                channel_->enableWriting();
            }
        }
    }

    void TcpConnection::shutdown()
    {
        auto selfPtr = shared_from_this();

        loop_->runInLoop([=]()
        {
            selfPtr->shutdownInLoop();
        });
    }

    void TcpConnection::shutdownInLoop()
    {
        loop_->assertInLoopThread();
        if (!channel_->isWriting())
        {
            SocketOps::ShutdownWrite(fd_);
        }
    }

    void TcpConnection::forceClose()
    {
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }

    void TcpConnection::forceCloseWithDelay(double seconds)
    {
        auto selfPtr = shared_from_this();
        loop_->runAfter(seconds, [=]()
        {
            selfPtr->forceClose();
        });
    }

    void TcpConnection::forceCloseInLoop()
    {
        loop_->assertInLoopThread();
        handleClose();        
    }

    void TcpConnection::setTcpNoDelay(bool on)
    {
        SocketOps::SetTcpNoDelay(fd_, on);
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
        loop_->assertInLoopThread();
        channel_->setOwner(selfPtr);
        channel_->enableReading();

        connectionCallback_(selfPtr);
    }

    void TcpConnection::connectDestroyed()
    {
        loop_->assertInLoopThread();
        channel_->disableAll();
        //connectionCallback_(shared_from_this());
       
        channel_->remove();
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
            sockErrorOccurred = true;
        }
    }

    void TcpConnection::handleWrite()
    {
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
            size_t n = SocketOps::Send(channel_->fd(),
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
                    

                    //shutdownInLoop();                    
                }
            }
            else
            {
                if (!ERR_RW_RETRIABLE(GetErrorCode()))
                {
                    sockErrorOccurred = true;
                    LOG_PRINT(LogType_Error, "TcpConnection::handleWrite");
                }
            }
        }
        else
        {
            LOG_PRINT(LogType_Info, "Connection fd = %d is down, no more writing", channel_->fd());
        }
    }

    void TcpConnection::handleClose()
    {
        loop_->assertInLoopThread();

        channel_->disableAll();
        sockErrorOccurred = true;
    }

    void TcpConnection::handleFinish()
    {
        if (sockErrorOccurred && closeCallback_)
        {
            closeCallback_(shared_from_this());
        }
    }
}