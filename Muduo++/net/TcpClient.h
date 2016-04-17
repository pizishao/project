#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "base/NonCopyable.h"
#include "net/TcpConnection.h"

namespace MuduoPlus
{
    class Connector;
    typedef std::shared_ptr<Connector> ConnectorPtr;

    class TcpClient : NonCopyable
    {
    public:
        // TcpClient(EventLoop* loop);
        // TcpClient(EventLoop* loop, const string& host, uint16_t port);
        TcpClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const std::string& nameArg);
        ~TcpClient();  // force out-line dtor, for scoped_ptr members.

        void connect();
        void disconnect();
        void stop();

        TcpConnectionPtr connection()
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            return connection_;
        }

        EventLoop* getLoop() const { return loop_; }
        bool retry() const { return retry_; }
        void enableRetry() { retry_ = true; }

        const std::string& name() const
        {
            return name_;
        }

        /// Set connection callback.
        /// Not thread safe.
        void setConnectionCallback(const ConnectionCallback& cb)
        {
            connectionCallback_ = cb;
        }

        /// Set message callback.
        /// Not thread safe.
        void setMessageCallback(const MessageCallback& cb)
        {
            messageCallback_ = cb;
        }

        /// Set write complete callback.
        /// Not thread safe.
        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {
            writeCompleteCallback_ = cb;
        }

    private:
        /// Not thread safe, but in loop
        void newConnection(int sockfd);
        /// Not thread safe, but in loop
        void removeConnection(const TcpConnectionPtr& conn);

        EventLoop* loop_;
        ConnectorPtr connector_; // avoid revealing Connector
        const std::string name_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool retry_;   // atomic
        bool connect_; // atomic
        // always in loop thread
        int nextConnId_;
        std::mutex mutex_;
        TcpConnectionPtr connection_; // @GuardedBy mutex_
    };

}