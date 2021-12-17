#include <tt/app/AppSystems.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

AppSystems::AppSystems()
{
}


AppSystems::~AppSystems()
{
}


snd::SoundSystemPtr AppSystems::instantiateSoundSystem()
{
	return snd::SoundSystemPtr();
}


PlatformApiPtr AppSystems::instantiatePlatformApi(PlatformCallbackInterface* /*p_platformCallbacks*/)
{
	return PlatformApiPtr();
}

// Namespace end
}
}
