// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TcpServer.h"
#include <stdio.h>

NetEventDispatcher dispatcher;
TcpServer srv(dispatcher);

void OnMessage(int64_t llClientHandle, void *pData, int32_t iLen)
{
    //printf("client[%lld]:msg[len=%d]:%s\n", llClientHandle, iLen, pData);
    srv.SendMessage(llClientHandle, pData, iLen);
}

void OnConnect(int64_t llClientHandle)
{
    printf("client[%lld] connected!\n", llClientHandle);
}

void OnClose(int64_t llClientHandle)
{
    printf("client[%lld] closed!\n", llClientHandle);
}

int _tmain(int argc, _TCHAR* argv[])
{
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD( 2, 2 );
    WSAStartup( wVersionRequested, &wsaData );
    
    srv.SetCloseCallback(OnClose);
    srv.SetConnectionCallback(OnConnect);
    srv.SetMessageCallback(OnMessage);
    InetAddress addr("192.168.1.101", 8000);
    
    srv.SetListenAddr(addr);
    srv.Start();
	dispatcher.Dispatch();

    getchar();

	return 0;
}

