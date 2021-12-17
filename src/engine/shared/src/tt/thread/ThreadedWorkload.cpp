#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

#include <tt/thread/thread.h>
#include <tt/thread/ThreadedWorkload.h>
#include <tt/thread/Semaphore.h>


namespace tt {
namespace thread {

//--------------------------------------------------------------------------------------------------
// Inner Threadpool class

struct ThreadPool
{
	static void create();
	static void destroy();
	static void setWorkload(const ThreadedWorkload& p_workload);
	static void waitForThreads();
	static void exitThreads();
	static void signalThreads();

	static int threadWorker(void*  p_index);

	typedef std::vector<tt::thread::handle>    ThreadHandles;

	static size_t                  ms_poolSize;
	static std::atomic<int>        ms_nextWorkIndex;
	static volatile bool           ms_exitWorkerThreads;
	static ThreadHandles           ms_threadPool;
	static tt::thread::Semaphore   ms_workDoneSemaphore;
	static ThreadedWorkload        ms_workload;
	static std::mutex              ms_conditionMutex;
	static std::condition_variable ms_condition;
};

size_t                    ThreadPool::ms_poolSize(0);
std::atomic<int>          ThreadPool::ms_nextWorkIndex(0);
volatile bool             ThreadPool::ms_exitWorkerThreads(false);
ThreadPool::ThreadHandles ThreadPool::ms_threadPool;
tt::thread::Semaphore     ThreadPool::ms_workDoneSemaphore;
ThreadedWorkload          ThreadPool::ms_workload;
std::mutex                ThreadPool::ms_conditionMutex;
std::condition_variable   ThreadPool::ms_condition;


//--------------------------------------------------------------------------------------------------
// Threadpool Methods

void ThreadPool::create()
{
	ms_poolSize = tt::thread::getProcessorCount();
	TT_ASSERT(ms_poolSize > 0);
	
	for (size_t i = 0; i < ms_poolSize; ++i)
	{
		char threadName[64];
		sprintf(threadName, "ThreadPool %d", static_cast<int>(i));
		const tt::thread::Affinity affinity(static_cast<tt::thread::Affinity>(tt::thread::Affinity_Core0 << i));
		void* intvalue(reinterpret_cast<void*>(i));
		ms_threadPool.push_back(tt::thread::create(&ThreadPool::threadWorker, intvalue, false, 0, tt::thread::priority_highest, affinity, threadName));
	}
}


void ThreadPool::destroy()
{
	TT_ASSERT(ms_threadPool.size() > 0);
	
	exitThreads();
	
	for (auto& it : ms_threadPool)
	{
		if (tt::thread::hasEnded(it) == false)
		{
			TT_PANIC("Failed to exit threadpool thread '%p'. Forcing exit", it.get());
			tt::thread::terminate(it, 0);
		}
	}
	ms_threadPool.clear();
	ms_poolSize = 0;
}


void ThreadPool::setWorkload(const ThreadedWorkload& p_workload)
{
	TT_ASSERT(ms_poolSize > 0);
	if (ms_workload.isEmpty() == false)
	{
		TT_PANIC("Still Threaded workload active. Waiting for completion.");
		waitForThreads();
		TT_ASSERT(ms_workload.isEmpty());
	}
	ms_workload = p_workload;
	ms_nextWorkIndex = 0;
	
	signalThreads();
}


void ThreadPool::exitThreads()
{
	ms_exitWorkerThreads = true;
	
	// Workload done; load an empty workload
	ms_workload = ThreadedWorkload();
	
	signalThreads();
	
	for (size_t i = 0; i < ms_poolSize; ++i)
	{
		tt::thread::wait(ms_threadPool[i]);
	}
}


void ThreadPool::signalThreads()
{
	std::lock_guard<std::mutex> lock(ms_conditionMutex);
	ms_condition.notify_all();
}


void ThreadPool::waitForThreads()
{
	for (size_t i = 0; i < ms_poolSize; ++i)
	{
		ms_workDoneSemaphore.wait();
	}
	
	// Workload done; load an empty workload
	ms_workload = ThreadedWorkload();
}


int ThreadPool::threadWorker(void* /*p_index*/)
{
	bool first = true;
	while (ms_exitWorkerThreads == false)
	{
		{
			std::unique_lock<std::mutex> lock(ms_conditionMutex);
			if (first == false)
			{
				ms_workDoneSemaphore.signal();
			}
			first = false;
			ms_condition.wait(lock);
		}
		
		if (ms_exitWorkerThreads)
		{
			break;
		}
		
		while (true)
		{
			const int nextWorkIndex = ms_nextWorkIndex.fetch_add(1);
			if (ms_workload.doWork(nextWorkIndex) == false)
			{
				break;
			}
		}
	}
	
	return 0;
}


//--------------------------------------------------------------------------------------------------
// ThreadedWorkload Methods

void ThreadedWorkload::createThreads()
{
	ThreadPool::create();
}


void ThreadedWorkload::destroyThreads()
{
	ThreadPool::destroy();
}


void ThreadedWorkload::startAndWaitForCompletion()
{
	if (isEmpty() == false)
	{
		ThreadPool::setWorkload(*this);
		ThreadPool::waitForThreads();
	}
}


// Namespace end
}
}
