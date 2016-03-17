#include "EventLoopThread.h"
#include "EventLoop.h"

namespace MuduoPlus
{
    EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
        : loop_(NULL),
        exiting_(false),
        /*threadPtr(std::bind(&EventLoopThread::threadFunc, this), name),*/
        mutex_(),
        callback_(cb)
    {
    }

    EventLoopThread::~EventLoopThread()
    {
        exiting_ = true;
        if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
        {
            // still a tiny chance to call destructed object, if threadFunc exits just now.
            // but when EventLoopThread destructs, usually programming is exiting anyway.
            loop_->quit();
            threadPtr->join();
        }
    }

    EventLoop* EventLoopThread::startLoop()
    {
        //assert(!threadPtr.started());
        threadPtr.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this)));
        //threadPtr.start();

        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            while (loop_ == NULL)
            {
                std::unique_lock<std::mutex> uniLock(mutex_);
                cond_.wait(uniLock);
            }
        }

        return loop_;
    }

    void EventLoopThread::threadFunc()
    {
        EventLoop loop;

        if (callback_)
        {
            callback_(&loop);
        }

        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            loop_ = &loop;
            cond_.notify_one();
        }

        loop.loop();
        //assert(exiting_);
        loop_ = NULL;
    }
}