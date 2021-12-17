#if !defined(INC_TT_APP_STEAMAPPSYSTEMS_H)
#define INC_TT_APP_STEAMAPPSYSTEMS_H

#include <tt/app/AppSystems.h>


namespace tt {
namespace app {

class SteamAppSystems : public AppSystems
{
public:
	virtual ~SteamAppSystems();
	
	virtual PlatformApiPtr instantiatePlatformApi(PlatformCallbackInterface* p_platformCallbacks);
};

// Namespace end
}
}

#endif  // !defined(INC_TT_APP_STEAMAPPSYSTEMS_H)
