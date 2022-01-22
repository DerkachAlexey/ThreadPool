#include "ThreadPool.hpp"
#include <iostream>

namespace tpl {

ThreadPool::ThreadPool() :
	// we suppose that our OS will tell us how many threads
	// are available, i.e. numberOfThreads always > 0
	m_numberOfThreads(std::thread::hardware_concurrency()),
	m_isRunning(true)
{
	initializeThreads();
}

ThreadPool::~ThreadPool()
{
	m_isRunning.store(false);

	m_threadsWaiter.notify_all();

	for (auto& thread : m_threads)
	{
		thread.join();
	}
}

void ThreadPool::initializeThreads()
{
	for (size_t i = 0; i < m_numberOfThreads; ++i)
	{
		m_threads.push_back(std::thread(&ThreadPool::dummyWorker, this));
	}
}

void ThreadPool::dummyWorker()
{
	// kill thread if running is false
	while (m_isRunning.load())
	{
		std::function<void()> runable;

		std::unique_lock<std::mutex> lock(m_dequeMutex);

		m_threadsWaiter.wait(lock, [this]() {return !m_tasks.empty() || !m_isRunning.load(); });

		// tasks queue may be empty after awakening since we use !m_isRunning.load()
		// in the predicate to escape dead lock when we join the thread and wait
		// for the conditional variable however there are no tasks
		if (!m_tasks.empty())
		{
			runable = m_tasks.front();
			m_tasks.pop_front();
		}

		lock.unlock(); // unlock to allow other threads perform tasks

		if (runable)
		{
			runable();
		}
	}
}

ThreadPool& ThreadPool::getInstance()
{
	static ThreadPool threadPool;
	return threadPool;
}

uint64_t ThreadPool::getNumberOfThreads() const
{
	return m_numberOfThreads;
}

} // namespace tpl
