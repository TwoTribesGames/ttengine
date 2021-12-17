#if !defined(INC_TT_THREAD_SEMAPHORE_H)
#define INC_TT_THREAD_SEMAPHORE_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {


class Semaphore
{
public:
	/*! \brief Create a semaphore.
	    \param p_initialCount The intial count for the semaphore. Needs to be >= 0. */
	explicit Semaphore(s32 p_initialCount = 0);
	~Semaphore();
	
	/*! \brief Decreases the count with one. If count is equal to zero the calling 
	           thread is blocked until it can decrease the semaphore count. */
	void wait();
	
	/*! \brief If the count is greater than zero, the count is decreased with one.
	           Otherwise the count isn't changed. (Does not block.)
	    \returns The previous count if decreasing was allowed. */
	bool tryWait();
	
	/*! \brief Increases the count with one.
	           If any threads are blocked while waiting one of them will be unblocked. */
	void signal();
	
	/*! \brief Sets a prefix for generated semaphore names, for platforms that require a name.
	           For Mac OS X sandboxed apps, set this to the "application group identifier"
	           specified in the app's entitlements file. */
	static void setNamePrefix(const std::string& p_prefix);
	
private:
	struct InternalData; // Forward declaration of the platform sepcific data. (Definition is in cpp file.)
	
	inline InternalData* getData() { return m_data; }
	
	Semaphore(const Semaphore&);                  // Disable copy
	const Semaphore& operator=(const Semaphore&); // Disable assigment.
	
	InternalData* m_data;
};


/*! \brief Wrapper class for Semaphore so it can be created at some later point. */
class OptionalSemaphore
{
public:
	inline OptionalSemaphore() : m_semaphore(0) {            }
	inline ~OptionalSemaphore()                 { destroy(); }
	
	inline void create(s32 p_initialCount = 0) { TT_ASSERT(m_semaphore == 0); destroy();
	                                             m_semaphore = new Semaphore(p_initialCount); }
	inline void destroy()                      { delete m_semaphore; m_semaphore = 0; }
	inline bool isValid() const                { return m_semaphore != 0; }
	
	inline void wait()    { TT_NULL_ASSERT(m_semaphore);        m_semaphore->wait();    }
	inline bool tryWait() { TT_NULL_ASSERT(m_semaphore); return m_semaphore->tryWait(); }
	inline void signal()  { TT_NULL_ASSERT(m_semaphore);        m_semaphore->signal();  }
	
private:
	Semaphore* m_semaphore;
};


// namespace end
}
}


#endif // !defined(INC_TT_THREAD_SEMAPHORE_H)
