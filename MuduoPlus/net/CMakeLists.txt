aux_source_directory(. net)
file(GLOB HEADER_FILES "*.h")

set(NET_SRCS
	Acceptor.cpp
	Buffer.cpp
	Channel.cpp
	Connector.cpp
	EventLoop.cpp
	EventLoopThread.cpp
	EventLoopThreadPool.cpp
	Poller.cpp
	SocketOps.cpp
	TcpClient.cpp
	TcpConnection.cpp
	TcpServer.cpp
	Timer.cpp
	TimerQueue.cpp
	HttpContext.cpp
	HttpResponse.cpp
	HttpServer.cpp
)

set(NET_HEADERS
	Acceptor.h
	Buffer.h
	CallBack.h
	Channel.h
	ChannelHolder.h
	Connector.h
	EventLoop.h
	EventLoopThread.h
	EventLoopThreadPool.h
	InetAddress.h
	Poller.h
	SocketOps.h
	TcpClient.h
	TcpConnection.h
	TcpServer.h
	Timer.h
	TimerId.h
	TimerQueue.h	
	HttpContext.h
	HttpRequest.h
	HttpResponse.h
	HttpServer.h
)

if(WIN32)
    set(NET_SRCS ${NET_SRCS} Selector.cpp)
	set(NET_HEADERS ${NET_HEADERS} Selector.h)
else()
	set(NET_SRCS ${NET_SRCS} Epoller.cpp)
	set(NET_HEADERS ${NET_HEADERS} Epoller.h)
endif()

ADD_LIBRARY(net ${NET_SRCS} ${NET_HEADERS})