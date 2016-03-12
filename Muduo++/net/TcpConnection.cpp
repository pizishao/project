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
        socket_(new Socket(sockfd)),
        channel_(new Channel(loop, sockfd)),
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
        channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError, this));
        socket_->setKeepAlive(true);
    }

    TcpConnection::~TcpConnection()
    {
        LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd();
    }

    bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
    {
        return socket_->getTcpInfo(tcpi);
    }

    std::string TcpConnection::getTcpInfoString() const
    {
        char buf[1024] = {0};
        socket_->getTcpInfoString(buf, sizeof buf);
        return buf;
    }

    void TcpConnection::send(const void* data, int len)
    {
        Buffer buf;
        buf.append(data, len);
        send(&buf);
    }

    /*void TcpConnection::send(const StringPiece& message)
    {
        if (state_ == kConnected)
        {
            if (loop_->isInLoopThread())
            {
                sendInLoop(message);
            }
            else
            {
                loop_->runInLoop(
                    std::bind(&TcpConnection::sendInLoop,
                    this,     // FIXME
                    message.as_string()));
                //std::forward<string>(message)));
            }
        }
    }*/

    // FIXME efficiency!!!
    void TcpConnection::send(Buffer* buf)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop,
                this,     // FIXME
                buf->retrieveAllAsString()));
            //std::forward<string>(message)));
        }
    }

    /*void TcpConnection::sendInLoop(const StringPiece& message)
    {
        sendInLoop(message.data(), message.size());
    }*/

    void TcpConnection::sendInLoop(const void* data, size_t len)
    {
        loop_->assertInLoopThread();
        ssize_t nwrote = 0;
        size_t remaining = len;
        bool faultError = false;

        // if no thing in output queue, try writing directly
        if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
        {
            nwrote = sockets::write(channel_->fd(), data, len);
            if (nwrote >= 0)
            {
                remaining = len - nwrote;
                if (remaining == 0 && writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
            }
            else // nwrote < 0
            {
                nwrote = 0;
                if (errno != EWOULDBLOCK)
                {
                    LOG_SYSERR << "TcpConnection::sendInLoop";
                    if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                    {
                        faultError = true;
                    }
                }
            }
        }

        assert(remaining <= len);
        if (!faultError && remaining > 0)
        {
            size_t oldLen = outputBuffer_.readableBytes();
            if (oldLen + remaining >= highWaterMark_
                && oldLen < highWaterMark_
                && highWaterMarkCallback_)
            {
                loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
            }
            outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
            if (!channel_->isWriting())
            {
                channel_->enableWriting();
            }
        }
    }

    void TcpConnection::shutdown()
    {
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));       
    }

    void TcpConnection::shutdownInLoop()
    {
        loop_->assertInLoopThread();
        if (!channel_->isWriting())
        {
            // we are not writing
            socket_->shutdownWrite();
        }
    }

    void TcpConnection::forceClose()
    {
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }

    void TcpConnection::forceCloseWithDelay(double seconds)
    {
        loop_->runAfter(
            seconds,
            makeWeakCallback(shared_from_this(),
            &TcpConnection::forceClose));  // not forceCloseInLoop to avoid race condition      
    }

    void TcpConnection::forceCloseInLoop()
    {
        loop_->assertInLoopThread();
        handleClose();        
    }

    void TcpConnection::setTcpNoDelay(bool on)
    {
        socket_->setTcpNoDelay(on);
    }

    void TcpConnection::startRead()
    {
        loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
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
        loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
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
        loop_->assertInLoopThread();
        channel_->tie(shared_from_this());
        channel_->enableReading();

        connectionCallback_(shared_from_this());
    }

    void TcpConnection::connectDestroyed()
    {
        loop_->assertInLoopThread();
        if (state_ == kConnected)
        {
            setState(kDisconnected);
            channel_->disableAll();

            connectionCallback_(shared_from_this());
        }
        channel_->remove();
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        loop_->assertInLoopThread();
        int savedErrno = 0;
        ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
        if (n > 0)
        {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
        else if (n == 0)
        {
            handleClose();
        }
        else
        {
            errno = savedErrno;
            LOG_SYSERR << "TcpConnection::handleRead";
            handleError();
        }
    }

    void TcpConnection::handleWrite()
    {
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
            ssize_t n = sockets::write(channel_->fd(),
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
                        
                    shutdownInLoop();                    
                }
            }
            else
            {
                LOG_SYSERR << "TcpConnection::handleWrite";
                // if (state_ == kDisconnecting)
                // {
                //   shutdownInLoop();
                // }
            }
        }
        else
        {
            LOG_TRACE << "Connection fd = " << channel_->fd()
                << " is down, no more writing";
        }
    }

    void TcpConnection::handleClose()
    {
        loop_->assertInLoopThread();
        LOG_TRACE << "fd = " << channel_->fd() << " state = ";

        channel_->disableAll();

        TcpConnectionPtr guardThis(shared_from_this());
        connectionCallback_(guardThis);
        // must be the last line
        closeCallback_(guardThis);
    }

    void TcpConnection::handleError()
    {
        int err = sockets::getSocketError(channel_->fd());
        LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
    }
}