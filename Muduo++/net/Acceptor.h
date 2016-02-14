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

    class Acceptor
    {
    public:
        typedef std::function < void(int sockfd,
            const InetAddress&) > NewConnCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();

        void SetNewConnCallBack(const NewConnCallback& cb)
        {
            m_NewConnCallBack = cb;
        }

        bool Listen();

    private:
        void HandleRead();

        bool                        m_bReuseport;
        InetAddress                 m_ListenAddr;

        EventLoop*                  m_pEventLoop;
        socket_t                    m_ListenFd;
        std::shared_ptr<Channel>    m_AcceptChannelPtr;
        NewConnCallback             m_NewConnCallBack;
    };
}