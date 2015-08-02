#include <iostream>

#include "Accepter.h"
#include "Sock.h"

#include <Mswsock.h>
#include <assert.h>

Accepter::Accepter()
{
    m_acceptOpContext.m_IoType = OP_Accept;
    m_receiveSocket=INVALID_SOCKET;
    memset(m_szBuffer, 0, POST_SIZE);
}

void Accepter::SetAttachIocpFunctor(AttachIocpFunc func)
{
    m_attachIocpFunctor=func;
}

void Accepter::SetAddConnFunctor(AddConnFunc func)
{
    m_addConnFunctor=func;
}

bool Accepter::PostPendAccept()
{
    m_receiveSocket = Sock::CreateSocket();
    if (m_receiveSocket == INVALID_SOCKET)
    {
        assert(false);
        return false;
    }    

    DWORD   dwLen=0;
    char    buffer[POST_SIZE]={0};

    if (!AcceptEx(m_listenSocket, m_receiveSocket, m_szBuffer, 0, sizeof(SOCKADDR_IN)+16, 
        sizeof(SOCKADDR_IN)+16, &dwLen, (LPOVERLAPPED)&m_acceptOpContext))
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            std::cout<<Sock::GetErrText();
            Sock::CloseFd(m_receiveSocket);
            return false;
        }
    } 

    return true;
}

bool Accepter::Listen(InetAddress &inetAddress)
{
    m_listenSocket = Sock::CreateSocket();
    if (INVALID_SOCKET == m_listenSocket)
    {
        std::cout<<Sock::GetErrText()<<std::endl;
        return false;
    }

    BOOL bReUseAddr = TRUE;
    if (!Sock::SetSockopt(m_listenSocket, SO_REUSEADDR, (char*)&bReUseAddr, sizeof(BOOL)))
    {
        std::cout<<Sock::GetErrText()<<std::endl;
        goto lblCleanUp;
    }

    if (!Sock::BindSocket(m_listenSocket,inetAddress))
    {
        std::cout<<Sock::GetErrText()<<std::endl;
        goto lblCleanUp;
    }

    if (!Sock::Listen(m_listenSocket))
    {
        std::cout<<Sock::GetErrText()<<std::endl;
        goto lblCleanUp;
    }

    if(!m_attachIocpFunctor(m_listenSocket, (ULONG_PTR)this))
    {
        std::cout<<Sock::GetErrText()<<std::endl;
        goto lblCleanUp;
    }


    if (!PostPendAccept())
    {
        goto lblCleanUp;
    }

    return true;

lblCleanUp:
    Sock::CloseFd(m_listenSocket);
    return false;
}

void Accepter::NotifyAccept(bool bSuccess)
{
    if (!bSuccess)
    {
        Sock::CloseFd(m_receiveSocket);
        PostPendAccept();

        return;
    }

    BOOL bOptValue = 1;
    if (setsockopt(m_receiveSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bOptValue, 
        sizeof(bOptValue)) != 0)
    {
        std::cout<<Sock::GetErrText();        
        goto lblError;
    }

    if (!Sock::SetSockopt(m_receiveSocket, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_listenSocket, 
        sizeof(m_listenSocket)))
    {
        std::cout<<Sock::GetErrText();        
        goto lblError;
    } 

    if (!m_addConnFunctor(m_receiveSocket))
    {
        std::cout<<Sock::GetErrText();        
        goto lblError;
    }

    PostPendAccept();
    return;

lblError:
    Sock::CloseFd(m_receiveSocket);
    PostPendAccept();
}

void Accepter::Close()
{
    Sock::CloseFd(m_receiveSocket);
    Sock::CloseFd(m_listenSocket);
}

