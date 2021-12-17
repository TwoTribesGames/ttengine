#if !defined(INC_TT_APP_APPSYSTEMS_H)
#define INC_TT_APP_APPSYSTEMS_H


#include <tt/app/PlatformApi.h>
#include <tt/platform/tt_types.h>
#include <tt/snd/types.h>


namespace tt {
namespace app {


class AppSystems
{
public:
	AppSystems();
	virtual ~AppSystems();
	
	/*! \brief Instantiates a default sound system.
	    \return A soundsystem or an empty shared pointer when no sound system is needed.*/
	virtual snd::SoundSystemPtr instantiateSoundSystem();
	
	/*! \brief Instantiates a platform API that will handle platform-specific details (e.g. Steam setup).
	    \param p_platformCallbacks Pointer to an instance that will handle platform callbacks. Does not take ownership. */
	virtual PlatformApiPtr instantiatePlatformApi(PlatformCallbackInterface* p_platformCallbacks);
	
	virtual inline void sdl_init()     {} // HACK Remove this once SDL is common in TTDEV.
	virtual inline void sdl_shutdown() {} // HACK Remove this once SDL is common in TTDEV.
	virtual inline void sdl_update()   {} // HACK Remove this once SDL is common in TTDEV.
};


typedef tt_ptr<AppSystems>::shared AppSystemsPtr;

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_APPSYSTEMS_H)
