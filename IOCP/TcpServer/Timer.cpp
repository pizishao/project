#include "Timer.h"

#include <assert.h>

Timer::Timer()
{
	m_iInterval = 0;
}

void Timer::SetPostFunctor(PostFunc func)
{
    m_postFunctor=func;
}

void Timer::Start(int32_t iInterval)
{
	assert(m_iInterval > 0);
    m_hTimerHandle = CreateWaitableTimer(0, FALSE, 0);

	LARGE_INTEGER dueTime;  
	dueTime.QuadPart=-iInterval * 1000 * 10;   
	SetWaitableTimer(m_hTimerHandle, &dueTime, 0, NULL, NULL, FALSE);
	
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
            m_postFunctor(PostOperation::Timer, 0);
        }
    }    
}
