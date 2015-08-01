#pragma once

#include "InetAddress.h"

#include <windows.h>
#include <functional>

typedef std::function<bool(SOCKET sk, ULONG_PTR CompletionKey)> AttachIocpFunc;
typedef std::function<bool(SOCKET sk)> AddConnFunc;

class Accepter
{
public:
    Accepter();
public:    
    void SetAttachIocpFunctor(AttachIocpFunc func);
    void SetAddConnFunctor(AddConnFunc func);    
    bool Listen(InetAddress &inetAddress);
    void NotifyAccept(bool bSuccess);
    bool PostPendAccept();
    void Close();

private:   

    char    m_szBuffer[POST_SIZE];

private:
    SOCKET              m_listenSocket;
    SOCKET              m_receiveSocket;

    OperateContext      m_acceptOpContext;
    AttachIocpFunc      m_attachIocpFunctor;
    AddConnFunc         m_addConnFunctor;    
};
