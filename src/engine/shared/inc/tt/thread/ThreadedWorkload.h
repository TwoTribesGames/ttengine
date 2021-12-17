#if !defined(INC_TT_THREAD_THREADEDWORKLOAD_H)
#define INC_TT_THREAD_THREADEDWORKLOAD_H

#include <functional>

namespace tt {
namespace thread {

class ThreadedWorkload
{
public:
	typedef std::function<void(size_t)> WorkCallback;
	
	static void createThreads();
	static void destroyThreads();
	
	ThreadedWorkload()
	:
	m_workSize(0),
	m_workCallback()
	{
	}
	
	ThreadedWorkload(size_t p_workSize, const WorkCallback& p_workCallback)
	:
	m_workSize(p_workSize),
	m_workCallback(p_workCallback)
	{
	}
	
	bool doWork(size_t p_index)
	{
		if (p_index < m_workSize)
		{
			m_workCallback(p_index);
			return true;
		}
		return false;
	}
	
	inline bool isEmpty() const { return m_workSize == 0; }
	
	void startAndWaitForCompletion();
	
private:
	size_t       m_workSize;
	WorkCallback m_workCallback;
};


// Namespace end
}
}


#endif // INC_TT_THREAD_THREADEDWORKLOAD_H
