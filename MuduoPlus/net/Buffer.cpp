// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "base/define.h"
#include "base/LinuxWin.h"
#include "Buffer.h"

namespace MuduoPlus
{

    const char Buffer::kCRLF[] = "\r\n";

    const size_t Buffer::kCheapPrepend;
    const size_t Buffer::kInitialSize;

    bool Buffer::readFd(int fd)
    {
#ifdef WIN32

        while(true)
        {
            char buf[MAX_TO_READ_ONCE] = { 0 };
            int n = SocketOps::secv(fd, buf, MAX_TO_READ_ONCE);

            if(n > 0)
            {
                append(buf, n);

                if(n < MAX_TO_READ_ONCE)  // no more data
                {
                    return true;
                }

                // go on read
            }
            else if(n == 0)  // connection lost
            {
                return false;
            }
            else
            {
                int errorCode = GetLastErrorCode();
                return ERR_RW_RETRIABLE(errorCode);
            }
        }

#else

        while(true)
        {
            char extrabuf[MAX_TO_READ_ONCE] = {0};
            struct iovec vec[2];
            const size_t writable = writableBytes();
            vec[0].iov_base = begin() + writerIndex_;
            vec[0].iov_len = writable;
            vec[1].iov_base = extrabuf;
            vec[1].iov_len = sizeof(extrabuf);

            const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
            const ssize_t n = readv(fd, vec, iovcnt);

            if(n < 0)
            {
                int errorCode = GetLastErrorCode();
                return ERR_RW_RETRIABLE(errorCode);
            }
            else if(n == 0)  // connection lost
            {
                return false;
            }
            else if(static_cast<size_t>(n) <= writable)
            {
                // no more data
                writerIndex_ += n;
                return true;
            }
            else
            {
                writerIndex_ = buffer_.size();
                append(extrabuf, n - writable);

                // go on read
            }
        }

#endif
    }

    /*bool Buffer::sendFd(int fd, int len)
    {
        while (true)
        {
        }
    }*/
}