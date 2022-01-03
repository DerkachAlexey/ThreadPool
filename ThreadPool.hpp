#include <thread>
#include <mutex>
#include <deque>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <memory>

#ifndef THREAD_POOL
#define THREAD_POOL

//tpl - thread pool library
namespace tpl {

	class ThreadPool
	{
		using Runable = std::function<void()>;

	public:
		static ThreadPool& getInstance();

		template<typename TFuncToRun, typename ...TFuncParams>
		void run(const TFuncToRun& funcToRun, TFuncParams&& ...funcParams)
		{
			std::unique_lock<std::mutex> lock(m_dequeMutex);

			m_tasks.push_back([&funcToRun, &funcParams...]()
			{
				funcToRun(std::forward<TFuncParams>(funcParams)...); 
			});

			lock.unlock();
			m_threadsWaiter.notify_one();
		}
		uint64_t getNumberOfThreads() const;

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&& movablePool) = delete;
		ThreadPool& operator = (const ThreadPool&) = delete;
		ThreadPool& operator = (ThreadPool&& movablePool) = delete;

		~ThreadPool();

	private:
		ThreadPool();

		void initializeThreads();
		void dummyWorker();

	private:
		size_t m_numberOfThreads;
		std::vector<std::thread> m_threads;
		std::deque<Runable> m_tasks;
		std::condition_variable m_threadsWaiter;
		std::mutex m_dequeMutex;
		std::atomic<bool> m_isRunning;
	};
}

#endif // !THREAD_POOL
