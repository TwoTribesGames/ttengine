#ifndef INC_TT_THREAD_BACKGROUNDTHREAD_H
#define INC_TT_THREAD_BACKGROUNDTHREAD_H


#include <SDL2/SDL_thread.h>
#include <stddef.h>

#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {

class BackgroundThread
{
public:
	/*! \brief Constructor, does not start the thread. Uses a default stacksize of 32K. */
	BackgroundThread(size_t p_stacksize = 32 * 1024);
	
	/*! \brief Destructor, does not stop the thread, please stop it first. */
	virtual ~BackgroundThread();
	
	/*! \brief Starts the thread. */
	void startThread(void* p_arg);
	
	/*! \brief Stops the thread. */
	int stopThread();
	
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
	
	
	SDL_Thread *m_thread;
	bool      m_active;
	bool      m_idle;
	
	void* m_arg;
	int   m_ret;
	
	size_t m_stacksize;
	u64*   m_stack;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_THREAD_BACKGROUNDTHREAD_H)
