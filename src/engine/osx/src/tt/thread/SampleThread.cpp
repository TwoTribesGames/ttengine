#include <tt/platform/tt_printf.h>
#include <tt/thread/SampleThread.h>


namespace tt {
namespace thread {

//--------------------------------------------------------------------------------------------------
// Public member functions

SampleThread::SampleThread()
:
BackgroundThread(),
m_running(true)
{
}


SampleThread::~SampleThread()
{
}


int SampleThread::run(void*)
{
	// As long as the thread is allowed to run, we can do whatever we want
	// keep concurrency in mind though
	s32 i = 0;
	while (m_running)
	{
		TT_Printf("%d: IM IN UR MAC, EATIN UR MEGAHURTZ\n", i);
		TT_Printf("  /\\_/\\ \n"
		          " ( o.o )\n"
		          "  > ^ < \n");
		++i;
	}
	return 0;
}


void SampleThread::stop()
{
	// Received a request from the creating thread to stop running
	m_running = false;
}

// Namespace end
}
}
