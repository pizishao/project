#pragma once

#include <memory>
#include <functional>

#include "Buffer.h"
#include "base/Timestamp.h"

namespace MuduoPlus
{
    class TcpConnection;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void()> TimerCallback;
    typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
    typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
    typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
    typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

// the data has been read to (buf, len)
    typedef std::function < void(const TcpConnectionPtr&,
                                 Buffer*, Timestamp) > MessageCallback;

    /*typedef std::function < void(int64_t clientHandle,
        Buffer*, Timestamp) > MessageCallback;*/

    void defaultConnectionCallback(const TcpConnectionPtr& conn);
    void defaultMessageCallback(const TcpConnectionPtr& conn,
                                Buffer* buffer,
                                Timestamp receiveTime);
}