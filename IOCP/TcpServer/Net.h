#pragma once

#include <WinSock2.h>
#include <stdint.h>
#include <functional>
#include <vector>
#include <memory>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#define POST_SIZE   (1 * 1024)

#define SAFE_CLOSE_HANDLE(handle)   {if (handle != nullptr){CloseHandle(handle);handle=nullptr;} }

typedef std::function<void(int64_t llClientHandle, void *pData, int32_t iLen)> MessageCallback;
typedef std::function<void(int64_t llClientHandle)> ConnectionCallback;
typedef std::function<void(int64_t llClientHandle)> CloseCallback;
typedef std::function<void()> TimerCallback;

typedef std::vector<char> Packet;
typedef std::shared_ptr<Packet> PacketPtr;
typedef std::vector<PacketPtr> PacketPtrList;

enum IocpOperation
{
    OP_Accept,
    OP_Send,
    OP_Recv,
    OP_Post,
};

struct PostOperation
{
    enum OpType
    {
        UserSendData,
        UserClose,
        UserQuit,
        Timer,
    };

    PostOperation()
    {
        llClientHandle = 0;
        type = UserSendData;
    }

    int64_t llClientHandle;
    OpType  type;
};

/*
struct UserOperation
{
    enum OpType
    {
        en_SendData,
        en_Close,
        en_Quit,
        en_Timer,
    };

    UserOperation()
    {
        llClientHandle = 0;
        type = en_SendData;
    }

    int64_t llClientHandle;
    OpType  type;
};*/

struct NetEvent
{
    enum Type
    {
        en_Connect,
        en_Msg,
        en_Close,
        en_Timer,
    };

    NetEvent()
    {
        m_llClientHandle = 0;
        m_eventType = en_Msg;
    }

    PacketPtr m_pktPtr;
    int64_t m_llClientHandle;
    Type m_eventType;

	MessageCallback		m_msgFunctor;
	ConnectionCallback	m_connFunctor;
	CloseCallback		m_closeFunctor;
    TimerCallback       m_timerCallbackFunctor;
};

class OperateContext: public WSAOVERLAPPED
{
public:
    OperateContext()
    {
        memset(this, 0, sizeof (WSAOVERLAPPED));
    }
public:
    IocpOperation m_IoType;
    WSABUF  m_wsaBuffer;
};
