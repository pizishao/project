#include "TcpClient.h"
#include "EventLoop.h"
#include "Connector.h"
#include "base/Logger.h"
#include "base/define.h"

namespace MuduoPlus
{
    namespace detail
    {
        void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
        {
            loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
        }

        void removeConnector(const ConnectorPtr& connector)
        {
            //connector->
        }
    }

    TcpClient::TcpClient(EventLoop* loop,
                         const InetAddress& serverAddr,
                         const std::string& nameArg)
        : loop_(loop),
          connector_(new Connector(loop, serverAddr)),
          name_(nameArg),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          retry_(false),
          connect_(true),
          nextConnId_(1)
    {
        connector_->setNewConnectionCallback(
            std::bind(&TcpClient::newConnection, this, std::placeholders::_1));

        // FIXME setConnectFailedCallback
        LOG_PRINT(LogType_Info, "TcpClient::TcpClient[%s] - connector %p",
                  name_.c_str(), connector_.get());
    }

    TcpClient::~TcpClient()
    {
        LOG_PRINT(LogType_Info, "TcpClient::~TcpClient[%s] - connector %p",
                  name_.c_str(), connector_.get());

        TcpConnectionPtr conn;
        bool unique = false;
        {
            LockGuarder(mutex_);
            unique = connection_.unique();
            conn = connection_;
        }

        if(conn)
        {
            assert(loop_ == conn->getLoop());
            // FIXME: not 100% safe, if we are in different thread
            CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
            loop_->runInLoop(
                std::bind(&TcpConnection::setCloseCallback, conn, cb));

            if(unique)
            {
                conn->forceClose();
            }
        }
        else
        {
            connector_->stop();
            // FIXME: HACK
            loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
        }
    }

    void TcpClient::connect()
    {
        // FIXME: check state
        LOG_PRINT(LogType_Info, "TcpClient::connect[%s] - connecting to %s",
                  name_.c_str(), connector_->serverAddress().toIpPort().c_str());

        connect_ = true;
        connector_->start();
    }

    void TcpClient::disconnect()
    {
        connect_ = false;

        {
            LockGuarder(mutex_);

            if(connection_)
            {
                connection_->gracefulClose();
            }
        }
    }

    void TcpClient::stop()
    {
        connect_ = false;
        connector_->stop();
    }

    void TcpClient::newConnection(int sockfd)
    {
        loop_->assertInLoopThread();
        InetAddress peerAddr(SocketOps::getPeerAddr(sockfd));
        char buf[32];
        snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
        ++nextConnId_;
        std::string connName = name_ + buf;

        InetAddress localAddr(SocketOps::getLocalAddr(sockfd));
        // FIXME poll with zero timeout to double confirm the new connection
        // FIXME use make_shared if necessary
        TcpConnectionPtr conn(new TcpConnection(loop_,
                                                connName,
                                                sockfd,
                                                localAddr,
                                                peerAddr));

        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setWriteCompleteCallback(writeCompleteCallback_);
        conn->setCloseCallback(
            std::bind(&TcpClient::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
        {
            LockGuarder(mutex_);
            connection_ = conn;
        }
        conn->connectEstablished();
    }

    void TcpClient::removeConnection(const TcpConnectionPtr& conn)
    {
        loop_->assertInLoopThread();
        assert(loop_ == conn->getLoop());

        {
            LockGuarder(mutex_);
            assert(connection_ == conn);
            connection_.reset();
        }

        loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

        if(retry_ && connect_)
        {
            LOG_PRINT(LogType_Info, "TcpClient::connect[%s] - Reconnecting to %s",
                      name_.c_str(), connector_->serverAddress().toIpPort().c_str());

            connector_->restart();
        }
    }
}