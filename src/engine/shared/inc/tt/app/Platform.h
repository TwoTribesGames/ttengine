#if !defined(INC_TT_APP_PLATFORM_H)
#define INC_TT_APP_PLATFORM_H

#include <tt/app/AppSettings.h>

namespace tt  /*! */ {
namespace app /*! */ {

#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
#define TT_SUPPORTS_PLATFORM_EMULATION 1
#else
#define TT_SUPPORTS_PLATFORM_EMULATION 0
#endif


/*! \brief Platform - Enumeration of possible software platforms */
enum Platform
{
	Platform_WIN, //<! Microsoft Windows   - PC
	Platform_MAC, //<! Apple Mac           - OSX.
	Platform_LNX, //<! Linux               - PC
	
	Platform_Count,
	Platform_Invalid
};


static inline bool isValidPlatform(Platform p_platform)
{ return p_platform >= 0 && p_platform < Platform_Count; }


#if TT_SUPPORTS_PLATFORM_EMULATION
void setPlatformEmulation(AppSettings::Emulation p_emulation);
#endif


Platform getPlatform(); //!< Set based on actual hardware, or emulation in AppSetting.

Platform getHostPlatform(); //!< Always based on actual hardware. (Use getPlatform() to get emulation.)


// TODO: Create a PlatformSpecs class. (put in own header.
/*
class PlatformSpecs
{
public:
	bool isDesktop();
	bool isDualScreen();
	InputType getInput(); // Something for mouse/touch/controller. maybe multiple are supported.
	
	// Should this be here? Or stay in TextureHardware?
	const TextureHardware::Requirements& getTextureRequirements();
};

// Is this a good idea? (host/target platforms?)
// Should it just be getPlatformSpecs for most code, with target and host for only specific situations?
const PlatformSpecs& getHostPlatformSpecs();
onst PlatformSpecs&  getTargetPlatformSpecs()
*/


// Namespace end
}
}


#endif  // !defined(INC_TT_APP_PLATFORM_H)
