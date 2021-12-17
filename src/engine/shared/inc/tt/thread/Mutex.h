#if !defined(INC_TT_THREAD_MUTEX_H)
#define INC_TT_THREAD_MUTEX_H

/*
	Mutex class, may be used to prevent multiple threads from accessing
	the same data/code simultaneously. Sample code below class declaration.
*/

namespace tt {
namespace thread {

class Mutex
{
public:
	Mutex();
	~Mutex();
	
	struct InternalData; //!< Holds internal platform specific mutex data.
	
	/*! \brief Locks the mutex.*/
	void lock();
	
	/*! \brief Tries to lock the mutex.
	    \return True when locked, false when not possible (already locked). */
	bool tryLock();
	
	/*! \brief Unlocks the mutex.*/
	void unlock();
	
	/*! \brief Returns the internal data of the mutex.
	    \return The internal data of the mutex.*/
	inline InternalData* getData() { return m_mutex; }
	
private:
	Mutex(const Mutex&);
	const Mutex& operator=(const Mutex&);
	
	InternalData* m_mutex;
};

// namespace end
}
}

/*
	Sample.
	
	static Container s_container;
	static Mutex     s_containerMutex;
	
	void addToContainer(Object p_object)
	{
		s_containerMutex.lock();
		s_container.add(p_object);
		s_container.unlock();
	}
	
	Object getFromContainer()
	{
		s_containerMutex.lock();
		Object object = s_container.get();
		s_containerMutex.unlock();
		return object;
	}
*/

#endif // !defined(INC_TT_THREAD_MUTEX_H)
