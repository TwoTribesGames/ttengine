#include <tt/platform/tt_error.h>

#include <tt/gwen/GwenMgr.h>


namespace tt {
namespace gwen {


GwenMgr* GwenMgr::ms_instance = 0;

//-------------------------------------------------------------------------------------------------
// Public

void GwenMgr::createInstance()
{
	TT_ASSERTMSG(ms_instance == 0, "Double creation of GwemMgr instance");
	ms_instance = new GwenMgr;
}


GwenMgr* GwenMgr::getInstance()
{
	TT_NULL_ASSERT(ms_instance);
	return ms_instance;
}


void GwenMgr::destroyInstance()
{
	TT_ASSERTMSG(ms_instance != 0, "Double deletion of GwenMgr instance");
	delete ms_instance;
	ms_instance = 0;
}


//-------------------------------------------------------------------------------------------------
// Private 

GwenMgr::GwenMgr()
{}


// Namespace end
}
}
