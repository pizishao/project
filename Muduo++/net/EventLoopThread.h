#pragma once

#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "base/NonCopyable.h"
#include "base/types.h"

namespace MuduoPlus
{
    class EventLoop;

    class EventLoopThread : NonCopyable
    {
    public:
        typedef std::function<void(EventLoop*)> ThreadInitCallback;

        EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();

        EventLoop*              loop_;
        bool                    exiting_;
        thread_ptr              threadPtr;
        std::mutex              mutex_;
        std::condition_variable cond_;
        ThreadInitCallback      callback_;
    };
}