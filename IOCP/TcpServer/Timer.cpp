#include "Timer.h"

#include <assert.h>

void Timer::SetPostFunctor(PostFunc func)
{
    m_postFunctor=func;
}

void Timer::Start(int32_t iMilliseconds)
{
    m_hTimerHandle = CreateWaitableTimer(0, FALSE, 0);
	m_iMilliseconds = iMilliseconds;

	LARGE_INTEGER dueTime;  
	dueTime.QuadPart=-m_iMilliseconds * 1000 * 10;   
	assert(SetWaitableTimer(m_hTimerHandle, &dueTime, 0, NULL, NULL, FALSE));
	
	m_bQuit = false;
    m_timerWaitThreadPtr = std::make_shared<std::thread>(&Timer::WaitLoop, this);    
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

			LARGE_INTEGER dueTime;  
			dueTime.QuadPart=-m_iMilliseconds * 1000 * 10; 
			assert(SetWaitableTimer(m_hTimerHandle, &dueTime, 0, NULL, NULL, FALSE));
        }
    }    
}
