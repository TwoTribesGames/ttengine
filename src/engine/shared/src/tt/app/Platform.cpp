
#include <tt/app/Platform.h>
#include <tt/platform/tt_error.h>

namespace tt {
namespace app {


#if TT_SUPPORTS_PLATFORM_EMULATION
Platform g_emulationPlatform = Platform_Invalid;


void setPlatformEmulation(AppSettings::Emulation p_emulation)
{
	switch (p_emulation)
	{
	case AppSettings::Emulate_None:
		g_emulationPlatform = Platform_WIN;
		break;
	default:
		TT_PANIC("Unknown emulation: %d\n", p_emulation);
		g_emulationPlatform = getHostPlatform();
		break;
	}
	
	TT_ASSERT(isValidPlatform(g_emulationPlatform));
}
#endif


Platform getPlatform()
{
#if TT_SUPPORTS_PLATFORM_EMULATION
	return (isValidPlatform(g_emulationPlatform)) ? g_emulationPlatform : getHostPlatform();
#else
	return getHostPlatform();
#endif
}


Platform getHostPlatform()
{
#if defined(TT_PLATFORM_WIN)
	return Platform_WIN;
#elif defined(TT_PLATFORM_OSX)
	return Platform_MAC; // Desktop Mac OS X
#elif defined(TT_PLATFORM_LNX)
	return Platform_LNX;
#else
	#error Unknown (new?) platform.
#endif
}


// Namespace end
}
}
