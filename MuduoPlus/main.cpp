// Andy.cpp : Defines the entry point for the console application.
//

#include <stdarg.h>

#include "base/Logger.h"
#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/TcpConnection.h"
#include "net/TcpClient.h"

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
        std::string s = buf->retrieveAllAsString();
        printf("on message:%s\n", s.c_str());
        conn->send(s.c_str(), s.size() + 1);
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

void onConnection(const MuduoPlus::TcpConnectionPtr& conn)
{
    printf("connection arrived!\n");
    /*LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");*/    
    conn->send("hellowrold", strlen("hellowrold"));
    conn->gracefulClose();
}

void onMessage(const MuduoPlus::TcpConnectionPtr& conn,
    MuduoPlus::Buffer* buf,
    MuduoPlus::Timestamp time)
{
    /*muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
    << "data received at " << time.toString();
    conn->send(msg);*/
    std::string s = buf->retrieveAllAsString();
    printf("on message:%s\n", s.c_str());
    conn->send(s.c_str(), s.size() + 1);
}

int main(int argc, char* argv[])
{
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
#endif    

    MuduoPlus::LogPrinter = LogPrintFunc;
    MuduoPlus::EventLoop loop;

    /*MuduoPlus::InetAddress listenAddr("0.0.0.0", 2007);
    EchoServer server(&loop, listenAddr);
    server.start();*/

    MuduoPlus::InetAddress serverAddr("120.25.100.182", 9527);
    MuduoPlus::TcpClient client(&loop, serverAddr, "lsy");
    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.connect();

    /*loop.runEvery(1, [=]()
    {
        printf("on timer\n");
    });*/
    loop.loop();
	return 0;
}

