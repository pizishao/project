// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TcpServer.h"
#include <stdio.h>

int _tmain(int argc, _TCHAR* argv[])
{    
    NetEventDispatcher dispatcher;
    TcpServer srv(dispatcher);

    srv.SetCloseCallback([=](int64_t llClientHandle)
    {
        printf("client[%lld] closed!\n", llClientHandle);
    });

    srv.SetConnectionCallback([=](int64_t llClientHandle)
    {
        printf("client[%lld] connected!\n", llClientHandle);
    });

    srv.SetMessageCallback([&srv](int64_t llClientHandle, void *pData, int32_t iLen)
    {
        srv.SendMessage(llClientHandle, pData, iLen);
    });

    srv.SetTimer(1000, []()
    {
        printf("OnTimer!\n");
    });

    InetAddress addr("192.168.0.2", 8000);
    
    srv.SetListenAddr(addr);
    srv.Start();
	dispatcher.Dispatch();

    getchar();

	srv.Stop();

	return 0;
}

