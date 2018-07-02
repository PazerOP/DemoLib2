#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
	Semaphore(uint_fast32_t initialCount) : m_Count(initialCount) {}

	void Notify()
	{
		std::lock_guard<decltype(m_Mutex)> lock(m_Mutex);
		m_Count++;
		m_ConditionVariable.notify_one();
	}

	void Wait()
	{
		std::unique_lock<decltype(m_Mutex)> lock(m_Mutex);

		while (!m_Count)
			m_ConditionVariable.wait(lock);

		m_Count--;
	}

	bool TryWait()
	{
		std::lock_guard<decltype(m_Mutex)> lock(m_Mutex);

		if (m_Count)
		{
			m_Count--;
			return true;
		}

		return false;
	}

private:
	std::mutex m_Mutex;
	std::condition_variable m_ConditionVariable;
	uint_fast32_t m_Count;
};

class SemaphoreAutoNotify
{
public:
	SemaphoreAutoNotify(Semaphore& semaphore) : m_Semaphore(semaphore)
	{
	}
	~SemaphoreAutoNotify()
	{
		m_Semaphore.Notify();
	}

private:
	Semaphore& m_Semaphore;
};