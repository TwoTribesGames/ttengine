#if !defined(INC_TT_THREAD_BACKGROUNDTHREAD_H)
#define INC_TT_THREAD_BACKGROUNDTHREAD_H


#include <tt/platform/tt_types.h>
#include <tt/thread/thread.h>
#include <tt/thread/types.h>


namespace tt {
namespace thread {


class BackgroundThread
{
public:
	/*! \brief Constructor, does not start the thread. Uses a default stack size of 64K. */
	BackgroundThread(size_t p_stackSize = 64 * 1024);
	
	/*! \brief Destructor, does not stop the thread, please stop it first. */
	virtual ~BackgroundThread();
	
	/*! \brief Starts the thread. */
	void startThread(void* p_arg, const char* p_threadName = 0);
	
	/*! \brief Stops the thread (waits for it to finish) */
	int stopThread();
	
	/*! \brief Set core affinity for the thread */
	void setAffinity(Affinity p_affinity);
	
	/*! \brief Get core affinity for the thread */
	Affinity getAffinity();
	
protected:
	/*! \brief Thread body, derived classes must implement this. */
	virtual int run(void* p_arg) = 0;
	
	/*! \brief Thread stopping signal, derived classes must implement this. */
	virtual void stop() = 0;
	
private:
	/*! \brief Runs the thread code */
	static int runThread(void* p_arg);
	
	// No copying
	BackgroundThread(const BackgroundThread&);
	BackgroundThread& operator=(const BackgroundThread&);
	
	handle         m_thread;
	
	bool           m_active;
	bool           m_idle;
	
	void*          m_arg;
	int            m_ret;
	
	const size_t   m_stackSize;
	Affinity       m_affinity;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_THREAD_BACKGROUNDTHREAD_H)
