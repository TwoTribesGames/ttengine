#include <tt/version/Version.h>

namespace tt {
namespace version {

static const s32   g_libRevisionNumber = 1;
static const char* g_libVersionName    = "1.0";

// should be set by client code
static s32         g_clientRevisionNumber = 0;
static const char* g_clientVersionName    = 0;


void setClientVersionInfo(s32         p_clientRevisionNumber,
                          const char* p_clientVersionName)
{
	g_clientRevisionNumber = p_clientRevisionNumber;
	g_clientVersionName    = p_clientVersionName;
}


s32 getLibRevisionNumber()
{
	return g_libRevisionNumber;
}


const char* getLibVersionName()
{
	return g_libVersionName;
}


s32 getClientRevisionNumber()
{
	return g_clientRevisionNumber;
}


const char* getClientVersionName()
{
	return g_clientVersionName;
}

// Namespace end
}
}
