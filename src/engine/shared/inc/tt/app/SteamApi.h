#if !defined(INC_TT_APP_STEAMAPI_H)
#define INC_TT_APP_STEAMAPI_H

#if defined(_MSC_VER)
#pragma warning( disable : 4127 ) // Disable conditional expression is constant
#endif

#include <steam/steam_api.h>

#include <tt/app/PlatformApi.h>
#include <tt/fs/types.h>


namespace tt {
namespace app {

class SteamApi : public PlatformApi
{
public:
	SteamApi(PlatformCallbackInterface* p_callbackInterface);
	virtual ~SteamApi();
	
	virtual bool init();
	virtual void shutdown();
	virtual void update();
	
private:
	void onSteamGameOverlayActivated(GameOverlayActivated_t* p_parameter);
	
	
	CCallbackManual<SteamApi, GameOverlayActivated_t, false> m_steamOverlayCallbacks;
	fs::FileSystemPtr m_cloudfs;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_APP_STEAMAPI_H)
