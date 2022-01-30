#include <thread>
#include <mutex>
#include <deque>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <future>
#include <tuple>

#ifndef THREAD_POOL
#define THREAD_POOL

//tpl - thread pool library
namespace tpl {

class ThreadPool
{
	using Runable = std::function<void()>;

public:
	static ThreadPool& getInstance();

	template<typename TFuncToRun, typename ...TFuncParams,
		typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<TFuncToRun>, std::decay_t<TFuncParams>...>>>>
	std::future<bool> run(TFuncToRun&& funcToRun, TFuncParams&& ...funcParams)
	{
		// we use bool to determine if the task is successfully completed
		auto sharedPtrPromise = std::make_shared<std::promise<bool>>();
		std::future<bool> funcFuture = sharedPtrPromise->get_future();

		std::unique_lock<std::mutex> lock(m_dequeMutex);
		m_tasks.push_back([funcToRun = std::forward<TFuncToRun>(funcToRun),
			funcParams = std::make_tuple(std::forward<TFuncParams>(funcParams)...), sharedPtrPromise]()
		{
			try
			{
				std::apply([&funcToRun](auto&& ... funcParams)
				{
					funcToRun(funcParams...);
				}, std::move(funcParams));
				
				sharedPtrPromise->set_value(true);
			}
			catch (...)
			{
				sharedPtrPromise->set_exception(std::current_exception());
			}
		});

		lock.unlock();
		m_threadsWaiter.notify_one();

		return funcFuture;
	}


	template<typename TFuncToRun, typename ...TFuncParams,
		typename TReturn = std::invoke_result_t<std::decay_t<TFuncToRun>, std::decay_t<TFuncParams>...>,
		typename = std::enable_if_t<!std::is_void_v<TReturn>>>
	std::future<TReturn> run(TFuncToRun&& funcToRun, TFuncParams&& ...funcParams)
	{
		auto sharedPtrPromise = std::make_shared<std::promise<TReturn>>();
		std::future<TReturn> funcFuture = sharedPtrPromise->get_future();

		std::unique_lock<std::mutex> lock(m_dequeMutex);

		m_tasks.push_back([funcToRun = std::forward<TFuncToRun>(funcToRun),
			funcParams = std::make_tuple(std::forward<TFuncParams>(funcParams)...), sharedPtrPromise]()
		{
			try
			{
				std::apply([&funcToRun, sharedPtrPromise](auto&& ... funcParams)
				{
					sharedPtrPromise->set_value(funcToRun(funcParams...));
				}, std::move(funcParams));
			}
			catch (...)
			{
				sharedPtrPromise->set_exception(std::current_exception());
			}
		});

		lock.unlock();
		m_threadsWaiter.notify_one();

		return funcFuture;
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