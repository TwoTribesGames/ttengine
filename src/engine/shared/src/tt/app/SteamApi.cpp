#include <string>

#include <tt/app/fatal_error.h>
#include <tt/app/PlatformCallbackInterface.h>
#include <tt/app/SteamApi.h>
#include <tt/fs/SteamFileSystem.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#if defined(TT_PLATFORM_WIN)
// To "fix" the unreachable code warning after reportFatalError 
#pragma warning(disable : 4702)
#endif

namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

SteamApi::SteamApi(PlatformCallbackInterface* p_callbackInterface)
:
PlatformApi(p_callbackInterface),
m_steamOverlayCallbacks(),
m_cloudfs()
{
}


SteamApi::~SteamApi()
{
}


bool SteamApi::init()
{
	// Steam initialization (Must occur before device creation)
	if (SteamAPI_Init() == false)
	{
		// FIXME: We could call SteamAPI_RestartAppIfNecessary before calling SteamAPI_Init
		
		TT_PANIC("Steam API failed to initialize. Check the following:\n"
		         "1) Steam must be running to play the game\n"
		         "2) The working directory must be set properly\n"
		         "3) The file steam_appid.txt with a valid appid must be present in the root\n"
		         "4) You must have a valid steam key for this app\n"
		         "5) Application must be running under the same user context as the Steam client, including admin privileges");
		
		reportFatalError("Steam client not running");
		
		return false;
	}
	
	// Register overlay callback
	m_steamOverlayCallbacks.Register(this, &SteamApi::onSteamGameOverlayActivated);
	
	// Init steam cloud fs
	// FIXME: How to pass whether to use cloud FS?
	bool p_useCloud = true;
	if (p_useCloud)
	{
		m_cloudfs = fs::SteamFileSystem::instantiate(1, SteamRemoteStorage());
		
#if !defined(TT_BUILD_FINAL)
		TT_Printf("\n\n--- Steam Cloud enabled ---\n");
		// do a little test
		const int32 count = SteamRemoteStorage()->GetFileCount();
		int32 totalSize = 0;
		TT_Printf("%d files available\n", count);
		for (int32 i = 0; i < count; ++i)
		{
			int32 size = 0;
			const char* file = SteamRemoteStorage()->GetFileNameAndSize(i, &size);
			totalSize += size;
			TT_Printf("- %d: '%s' (%d bytes)\n", i, file, size);
		}
		TT_Printf("Total storage used: %d bytes (%.2f KiB)\n", totalSize, totalSize / 1024.0f);
		TT_Printf("---------------------------\n\n");
#endif
	}
	else
	{
		// Use regular FS for steam
		m_cloudfs = fs::SteamFileSystem::instantiate(1, 0);
	}
	
	return true;
}


void SteamApi::shutdown()
{
	m_cloudfs.reset();
	SteamAPI_Shutdown();
}


void SteamApi::update()
{
	SteamAPI_RunCallbacks();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SteamApi::onSteamGameOverlayActivated(GameOverlayActivated_t* p_parameter)
{
	if (p_parameter->m_bActive)
	{
		getCallbackInterface()->onPlatformMenuEnter();
	}
	else
	{
		getCallbackInterface()->onPlatformMenuExit();
	}
}

// Namespace end
}
}
