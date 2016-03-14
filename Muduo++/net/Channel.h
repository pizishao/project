#pragma once

#include <string>
#include <functional>
#include <memory>

namespace MuduoPlus
{
    class EventLoop;

    class Channel
    {
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void(Timestamp)> ReadEventCallback;

        Channel(EventLoop* loop, int fd);
        ~Channel();

        void handleEvent(Timestamp receiveTime);

        void setReadCallback(const ReadEventCallback& cb)
        {
            readCallback_ = cb;
        }

        void setWriteCallback(const EventCallback& cb)
        {
            writeCallback_ = cb;
        }

        void setCloseCallback(const EventCallback& cb)
        {
            closeCallback_ = cb;
        }

        void setFinishCallback(const EventCallback& cb)
        {
            finishCallback_ = cb;
        }

        int  fd() const { return fd_; }
        int  interestEvents() const { return interestEvents_; }
        void setRecvEvents(int revt) { recvEvents_ = revt; } // used by pollers
        bool isNoneEvent() const { return interestEvents_ == kNoneEvent; }

        void enableReading() { interestEvents_ |= kReadEvent; update(); }
        void disableReading() { interestEvents_ &= ~kReadEvent; update(); }
        void enableWriting() { interestEvents_ |= kWriteEvent; update(); }
        void disableWriting() { interestEvents_ &= ~kWriteEvent; update(); }
        void disableAll() { interestEvents_ = kNoneEvent; update(); }
        bool isWriting() const { return interestEvents_ & kWriteEvent; }
        bool isReading() const { return interestEvents_ & kReadEvent; }

        // for Poller
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        EventLoop* ownerLoop() { return loop_; }
        void remove();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
        static const int kCloseEvent;

    private:

        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        EventLoop* loop_;
        const int  fd_;
        int        interestEvents_;
        int        recvEvents_; // it's the received event types of epoll or select
        int        index_;      // used by Poller.

        bool addedToLoop_;

        ReadEventCallback   readCallback_;
        EventCallback       writeCallback_;
        EventCallback       closeCallback_;
        EventCallback       finishCallback_;
    };
}