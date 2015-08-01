#pragma once

#include <mutex>               
#include <condition_variable>
#include <deque>
#include <assert.h>

template<typename T>
class BlockingQueue
{
public:
	BlockingQueue()
	{
		m_bQuit = false;
	}

	void put(T &t)
	{
		std::lock_guard<std::mutex> lockGuard(m_Lock);
		m_queue.push_back(t);
		m_Cond.notify_one();
	}

	T take()
	{
		std::unique_lock <std::mutex> uniqueLock(m_Lock);

		if (m_bQuit)
		{
			return;
		}

		while (m_queue.empty())
		{
			m_Cond.wait(uniqueLock);

			if (m_bQuit)
			{
				return;
			}
		}

		assert(!m_queue.empty());

		T front(m_queue.front());

		m_queue.pop_front();
		return front;
	}

	void take(std::deque<T> &queue)
	{
		std::unique_lock <std::mutex> uniqueLock(m_Lock);

		while (m_queue.empty())
		{
			m_Cond.wait(uniqueLock);

			if (m_bQuit)
			{
				return;
			}
		}

		assert(!m_queue.empty());

		queue.swap(m_queue);
	}

	size_t size() const
	{
		std::lock_guard<std::mutex> lockGuard(m_Lock);
		return m_queue.size();
	}

	void Quit()
	{
		m_bQuit = true;
		m_Cond.notify_one();
	}

private:
	std::mutex					  m_Lock;	
	std::condition_variable       m_Cond;
	std::deque<T>				  m_queue;

	bool						  m_bQuit;
};
