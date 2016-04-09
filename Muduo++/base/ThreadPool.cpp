#include <algorithm>

#include "LinuxWin.h"
#include "ThreadPool.h"

namespace MuduoPlus
{
    ThreadPool::ThreadPool(const std::string& nameArg)
        :name_(nameArg),
        maxQueueSize_(0),
        running_(false)
    {
    }

    ThreadPool::~ThreadPool()
    {
        if (running_)
        {
            stop();
        }
    }

    void ThreadPool::start(int numThreads)
    {
        assert(threads_.empty());
        running_ = true;

        threads_.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            char id[32];
            snprintf(id, sizeof id, "%d", i + 1);

            threads_.push_back(std::make_shared<std::thread>(std::bind(&ThreadPool::runInThread, this)));
        }

        if (numThreads == 0 && threadInitCallback_)
        {
            threadInitCallback_();
        }
    }

    void ThreadPool::stop()
    {
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            running_ = false;
            notEmpty_.notify_all();
        }

        for (auto &pos : threads_)
        {
            pos->join();
        }
    }

    size_t ThreadPool::queueSize()
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        return queue_.size();
    }

    void ThreadPool::run(const Task& task)
    {
        if (threads_.empty())
        {
            task();
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(mutex_);
            while (isFull())
            {
                std::unique_lock<std::mutex> uniLock(mutex_);
                notFull_.wait(uniLock);
            }
            assert(!isFull());

            queue_.push_back(task);
            notEmpty_.notify_one();
        }
    }

    ThreadPool::Task ThreadPool::take()
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        // always use a while-loop, due to spurious wakeup
        while (queue_.empty() && running_)
        {
            std::unique_lock<std::mutex> uniLock(mutex_);
            notEmpty_.wait(uniLock);
        }

        Task task;
        if (!queue_.empty())
        {
            task = queue_.front();
            queue_.pop_front();
            if (maxQueueSize_ > 0)
            {
                notFull_.notify_one();
            }
        }
        return task;
    }

    bool ThreadPool::isFull() const
    {
        //mutex_.assertLocked();
        return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
    }

    void ThreadPool::runInThread()
    {
        try
        {
            if (threadInitCallback_)
            {
                threadInitCallback_();
            }
            while (running_)
            {
                Task task(take());
                if (task)
                {
                    task();
                }
            }
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
            throw; // rethrow
        }
    }
}