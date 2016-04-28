#pragma once

#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <assert.h>
#include <vector>
#include <memory>

#include "NonCopyable.h"
#include "base/types.h"

namespace MuduoPlus
{
    class ThreadPool : NonCopyable
    {
    public:
        typedef std::function<void()> Task;

        explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
        ~ThreadPool();

        // Must be called before start().
        void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
        void setThreadInitCallback(const Task& cb)
        {
            threadInitCallback_ = cb;
        }

        void start(int numThreads);
        void stop();

        const std::string& name() const
        {
            return name_;
        }

        size_t queueSize();

        // Could block if maxQueueSize > 0
        void run(const Task& f);

    private:
        bool isFull() const;
        void runInThread();
        Task take();

        std::mutex mutex_;
        std::condition_variable  notEmpty_;
        std::condition_variable  notFull_;
        std::string name_;
        Task threadInitCallback_;
        vector_ptr<std::thread > threads_;
        std::deque<Task> queue_;
        size_t maxQueueSize_;
        bool running_;
    };
}