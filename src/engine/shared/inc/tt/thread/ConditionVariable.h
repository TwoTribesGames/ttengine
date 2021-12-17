#if !defined(INC_TT_THREAD_CONDITIONVARIABLE_H)
#define INC_TT_THREAD_CONDITIONVARIABLE_H

/*
	ConditionVariable class, provides atomic unlocking of mutex and waiting for condition.
	Sample code below class declaration.
*/

namespace tt {
namespace thread {

// forward declaration
class Mutex;

class ConditionVariable
{
public:
	ConditionVariable();
	~ConditionVariable();
	
	/*! \brief Sleeps on the condition variable and release the specified mutex.
	    \param p_mutex The mutex to release */
	void sleep(Mutex* p_mutex);
	
	/*! \brief Wakes all threads waiting on this condition variable. */
	void wake();
	
private:
	ConditionVariable(const ConditionVariable&);
	const ConditionVariable& operator=(const ConditionVariable&);
	
	struct InternalData;
	
	InternalData* m_variable;
};


// Namespace end
}
}


/*
	Sample.
	Consumer/Producer problem
	
	static Queue s_queue;
	static Mutex s_queueMutex;
	static ConditionVariable s_notEmpty;
	static ConditionVariable s_notFull;
	
	void producerThread()
	{
		for (;;)
		{
			s_queueMutex.lock();
			while (s_queue.full())
			{
				s_notFull.wait(&s_queueMutex); // wait for consumers to consume
			}
			s_queue.push_front(Object());
			s_queueMutex.unlock();
			s_notEmpty.wake(); // notify any waiting consumers
		}
	}
	
	void consumerThread()
	{
		for (;;)
		{
			s_queueMutex.lock();
			while (s_queue.empty())
			{
				s_notEmpty.wait(&s_queueMutex); // wait for producers to produce
			}
			s_queue.pop_back();
			s_queueMutex.unlock();
			s_notFull.wake(); // notify any waiting producers
		}
	}
*/

#endif // !defined(INC_TT_THREAD_CONDITIONVARIABLE_H)
