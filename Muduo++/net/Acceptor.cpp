#include "Acceptor.h"

namespace MuduoPlus
{


    Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    {
        m_EventLoop     = loop;
        m_ListenAddr    = listenAddr;
        m_bReuseport    = reuseport;
    }

    Acceptor::~Acceptor()
    {

    }

    bool Acceptor::listen()
    {
        socket_t fd = CreateSocket();
        if (fd == -1)
        {
            return false;
        }


    }

    void Acceptor::handleRead()
    {

    }

}