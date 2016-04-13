// Andy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "base/Logger.h"
#include "net/TcpServer.h"
#include "net/EventLoop.h"

#pragma comment(lib, "ws2_32.lib")

class EchoServer
{
public:
    EchoServer(MuduoPlus::EventLoop* loop,
        const MuduoPlus::InetAddress& listenAddr)
        : server_(loop, listenAddr, "EchoServer")
    {
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this,std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const MuduoPlus::TcpConnectionPtr& conn)
    {
        printf("connection arrived!\n");
        /*LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");*/
    }

    void onMessage(const MuduoPlus::TcpConnectionPtr& conn,
        MuduoPlus::Buffer* buf,
        MuduoPlus::Timestamp time)
    {
        /*muduo::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
            << "data received at " << time.toString();
        conn->send(msg);*/
        printf("on message!\n");
    }

    MuduoPlus::TcpServer server_;
};

void LogPrintFunc(MuduoPlus::LogType type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    printf("\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);

    MuduoPlus::LogPrinter = LogPrintFunc;
    MuduoPlus::EventLoop loop;
    MuduoPlus::InetAddress listenAddr("192.168.0.2", 2007);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
	return 0;
}

