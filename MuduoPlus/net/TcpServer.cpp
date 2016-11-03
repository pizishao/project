#include <stdio.h>
#include <assert.h>

#include "TcpServer.h"
#include "TcpConnection.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketOps.h"
#include "EventLoopThreadPool.h"
#include "base/Logger.h"

namespace MuduoPlus
{
    TcpServer::TcpServer(EventLoop* loop,
                         const InetAddress& listenAddr,
                         const std::string& nameArg,
                         Option option)
        : loop_(loop),
          ipPort_(listenAddr.toIpPort()),
          name_(nameArg),
          acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
          threadPool_(new EventLoopThreadPool(loop, name_)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nextConnId_(1)
    {
        acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
        started_ = 0;
    }

    TcpServer::~TcpServer()
    {
        loop_->assertInLoopThread();
        LOG_PRINT(LogType_Info, "TcpServer::~TcpServer [%s] destructing", name_.c_str());

        for(ConnectionMap::iterator it(connections_.begin());
                it != connections_.end(); ++it)
        {
            TcpConnectionPtr conn = it->second;
            it->second.reset();
            conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
            conn.reset();
        }
    }

    void TcpServer::setThreadNum(int numThreads)
    {
        assert(0 <= numThreads);
        threadPool_->setThreadNum(numThreads);
    }

    void TcpServer::start()
    {
        if(started_.fetch_and(1) == 0)
        {
            threadPool_->start(threadInitCallback_);
            assert(!acceptor_->listenning());
            loop_->runInLoop(
                std::bind(&Acceptor::listen, acceptor_.get()));
        }
    }

    void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
    {
        loop_->assertInLoopThread();
        EventLoop* ioLoop = threadPool_->getNextLoop();
        char buf[64];
        snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
        ++nextConnId_;
        std::string connName = name_ + buf;

        LOG_PRINT(LogType_Info, "TcpServer::newConnection [%s] - new connection [%s] from %s",
                  name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
        InetAddress localAddr(SocketOps::getLocalAddr(sockfd));
        // FIXME poll with zero timeout to double confirm the new connection
        // FIXME use make_shared if necessary
        TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                                connName,
                                                sockfd,
                                                localAddr,
                                                peerAddr));
        connections_[connName] = conn;
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setWriteCompleteCallback(writeCompleteCallback_);
        conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
        ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    }

    void TcpServer::removeConnection(const TcpConnectionPtr& conn)
    {
        // FIXME: unsafe
        loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
    {
        loop_->assertInLoopThread();
        LOG_PRINT(LogType_Info, "TcpServer::removeConnectionInLoop [%s] - connection %s",
                  name_.c_str(), conn->name().c_str());
        size_t n = connections_.erase(conn->name());
        (void)n;
        assert(n == 1);
        EventLoop* loop = conn->getLoop();
        loop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}