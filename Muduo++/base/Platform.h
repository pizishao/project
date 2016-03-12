#pragma once

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/in6.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

#ifdef WIN32
#define socket_t intptr_t
#else
#define socket_t int
#endif

#ifndef WIN32

/* True iff e is an error that means a read/write operation can be retried. */
#define ERR_RW_RETRIABLE(e)				\
	((e) == EINTR || (e) == EAGAIN)
/* True iff e is an error that means an connect can be retried. */
#define ERR_CONNECT_RETRIABLE(e)			\
	((e) == EINTR || (e) == EINPROGRESS)
/* True iff e is an error that means a accept can be retried. */
#define ERR_ACCEPT_RETRIABLE(e)			\
	((e) == EINTR || (e) == EAGAIN || (e) == ECONNABORTED)

/* True iff e is an error that means the connection was refused */
#define ERR_CONNECT_REFUSED(e)					\
	((e) == ECONNREFUSED)

#else

#define ERR_RW_RETRIABLE(e)					\
	((e) == WSAEWOULDBLOCK ||					\
	    (e) == WSAEINTR)

#define ERR_CONNECT_RETRIABLE(e)					\
	((e) == WSAEWOULDBLOCK ||					\
	    (e) == WSAEINTR ||						\
	    (e) == WSAEINPROGRESS ||					\
	    (e) == WSAEINVAL)

#define ERR_ACCEPT_RETRIABLE(e)			\
	    ERR_RW_RETRIABLE(e)

#define ERR_CONNECT_REFUSED(e)					\
	((e) == WSAECONNREFUSED)

#endif

#define MAX_TO_READ_EVER    (16384)
#define MAX_TO_WRITE_EVER   (16384)

enum ResultCode
{
    success,
    fail,
    retry,
};
