#ifndef INC_TT_THREAD_SAMPLETHREAD_H
#define INC_TT_THREAD_SAMPLETHREAD_H


#include <tt/thread/BackgroundThread.h>


namespace tt {
namespace thread {

class SampleThread : public BackgroundThread
{
public:
	SampleThread();
	virtual ~SampleThread();
	
	virtual int run(void* p_arg);
	virtual void stop();
	
private:
	bool m_running;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_THREAD_SAMPLETHREAD_H)
