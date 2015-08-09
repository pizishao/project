#include "Timer.h"

#include <assert.h>

void Timer::SetPostFunctor(PostFunc func)
{
    m_postFunctor=func;
}

void Timer::Start()
{
    m_hTimerHandle = CreateWaitableTimer(0, FALSE, 0);
    m_timerWaitThreadPtr = std::make_shared<std::thread>(&Timer::WaitLoop, this);
    m_bQuit = false;
}

void Timer::Stop()
{
    m_bQuit=true;
    m_timerWaitThreadPtr->join();
    m_timerWaitThreadPtr.reset();
    CloseHandle(m_hTimerHandle);
}

void Timer::WaitLoop()
{
    assert(m_postFunctor);

    while (!m_bQuit)
    {
        if (::WaitForSingleObject(m_hTimerHandle, INFINITE) == WAIT_OBJECT_0)
        {
            m_postFunctor(PostOperation::Timer);
        }
    }    
}
