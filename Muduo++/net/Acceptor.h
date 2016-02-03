#pragma once

#include <functional>
#include <memory>

#include "base/Type.h"
#include "Channel.h"
#include "InetAddress.h"

namespace MuduoPlus
{
    class EventLoop;
    class InetAddress;

    class Acceptor
    {
    public:
        typedef std::function < void(int sockfd,
            const InetAddress&) > NewConnectionCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {
            m_NewConnCallBack = cb;
        }

        bool listen();

    private:
        void handleRead();

        bool                        m_bReuseport;
        InetAddress                 m_ListenAddr;

        EventLoop*                  m_EventLoop;
        socket_t                    m_ListenFd;
        std::shared_ptr<Channel>    m_AcceptChannelPtr;
        NewConnectionCallback       m_NewConnCallBack;
    };
}