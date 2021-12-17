#if !defined(INC_TT_THREAD_CRITICALSECTION_H)
#define INC_TT_THREAD_CRITICALSECTION_H

#include <tt/platform/tt_error.h>
#include <tt/thread/Mutex.h>

/*
	CriticalSection class, provides automatic (un)locking of a mutex.
	Sample code below class declaration.
*/

namespace tt {
namespace thread {

// forward declaration
class Mutex;

class CriticalSection
{
public:
	/*! \brief Constructor, immediately enters critical section. */
	CriticalSection(Mutex* p_mutex)
	:
	m_mutex(p_mutex)
	{
		TT_ASSERTMSG(m_mutex != 0, "No mutex specified!");
		m_mutex->lock();
	}
	
	/*! \brief Destructor, leaves critical secion. */
	~CriticalSection()
	{
		m_mutex->unlock();
	}
	
private:
	CriticalSection(const CriticalSection&);
	const CriticalSection& operator=(const CriticalSection&);
	
	Mutex* m_mutex; //!< Mutex object used for mutual exclusion.
};


// Namespace end
}
}

/*
	Sample.
	
	static Container s_container;
	static Mutex     s_containerMutex;
	
	void addToContainer(Object p_object)
	{
		CriticalSection section(&s_containerMutex);
		s_container.add(p_object);
	}
	
	Object getFromContainer()
	{
		CriticalSection section(&s_containerMutex);
		return s_container.get();
	}
*/

#endif // !defined(INC_TT_THREAD_CRITICALSECTION_H)
