#pragma once

#include <functional>
#include <memory>

#include "base/Platform.h"
#include "Channel.h"
#include "InetAddress.h"

namespace MuduoPlus
{
    class EventLoop;
    class InetAddress;

    class Acceptor : public std::enable_shared_from_this<Acceptor>
    {
    public:
        typedef std::function < void(int sockfd,
            const InetAddress&) > NewConnCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnCallback& cb)
        {
            newConnCallBack_ = cb;
        }

        void listen();
        bool listenning() const { return listenning_; }

    private:
        void handleRead();

        bool                        isReuseport_;
        InetAddress                 listenAddr_;
        bool                        listenning_;
        EventLoop*                  loop_;
        socket_t                    listenFd_;
        std::shared_ptr<Channel>    acceptChannelPtr_;
        NewConnCallback             newConnCallBack_;
    };
}