#include <tt/app/SteamApi.h>
#include <tt/app/SteamAppSystems.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

SteamAppSystems::~SteamAppSystems()
{
}


PlatformApiPtr SteamAppSystems::instantiatePlatformApi(PlatformCallbackInterface* p_platformCallbacks)
{
	return PlatformApiPtr(new SteamApi(p_platformCallbacks));
}

// Namespace end
}
}
