#pragma once

#include "Net.h"

#include <Windows.h>
#include <memory>
#include <thread>

typedef std::function<void(PostOperation::OpType iOpCode, int64_t llClientHandle)> PostFunc;

class Timer
{
public:
    void SetPostFunctor(PostFunc func);
    void Start(int32_t iMilliseconds);
    void Stop();	

private:
    void WaitLoop();

private:
    HANDLE m_hTimerHandle;
    std::shared_ptr<std::thread> m_timerWaitThreadPtr;
	int32_t m_iMilliseconds;
    PostFunc m_postFunctor;
    bool m_bQuit;
};