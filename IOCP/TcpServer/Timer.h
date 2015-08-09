#pragma once

#include "Net.h"

#include <Windows.h>
#include <memory>
#include <thread>

typedef std::function<void(PostOperation::OpType iOpCode)> PostFunc;

class Timer
{
public:
    void SetPostFunctor(PostFunc func);
    void Start();
    void Stop();

private:
    void WaitLoop();
private:
    HANDLE m_hTimerHandle;
    std::shared_ptr<std::thread> m_timerWaitThreadPtr;
    PostFunc m_postFunctor;
    bool m_bQuit;
};