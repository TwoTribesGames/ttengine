#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#include <tt/app/ComHelper.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace app {

s32  ComHelper::ms_initializedCount = 0;
bool ComHelper::ms_multithreaded    = true;


//--------------------------------------------------------------------------------------------------
// Public member functions

void ComHelper::initCom()
{
	TT_ASSERTMSG(ms_initializedCount >= 0, "ComHelper::uninitCom was called too many times.");
	if (ms_initializedCount == 0)
	{
		// First request for COM: initialize
		HRESULT hr = CoInitializeEx(0, ms_multithreaded ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);
		// NOTE: CoInitializeEx returns S_FALSE if called more than once (not a real error)
		TT_ASSERTMSG(hr == S_FALSE || SUCCEEDED(hr),
		             "Initializing COM failed! Error code: 0x%08X", hr);
	}
	
	++ms_initializedCount;
}


void ComHelper::uninitCom()
{
	--ms_initializedCount;
	
	if (ms_initializedCount == 0)
	{
		// Nobody wants COM anymore: uninitialize
		CoUninitialize();
	}
	
	TT_ASSERTMSG(ms_initializedCount >= 0, "ComHelper::uninitCom was called too many times.");
}


void ComHelper::setMultiThreaded(bool p_multithreaded)
{
	TT_ASSERTMSG(ms_initializedCount <= 0,
	             "COM is already initialized; changing threading mode has no effect.");
	ms_multithreaded = p_multithreaded;
}

// Namespace end
}
}
