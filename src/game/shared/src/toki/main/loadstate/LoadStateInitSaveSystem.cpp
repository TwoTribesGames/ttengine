#include <toki/audio/AudioPlayer.h>
#include <toki/main/loadstate/LoadStateInitSaveSystem.h>
#include <toki/savedata/utils.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateInitSaveSystem::doLoadStep()
{
	savedata::initSaveUtils();
	
	if (savedata::mountSaveVolumeForReading())
	{
		audio::AudioPlayer::loadVolumeSettings();
		
		input::Controller::loadCustomKeyBindings();
		
		savedata::unmountSaveVolume();
	}
	
	// Now that we have correct volume settings, start playing the loading music
	AppGlobal::getSharedGraphics().startBackgroundMusic();
}


bool LoadStateInitSaveSystem::isDone() const
{
	// This state only has one step: done right away
	return true;
}

// Namespace end
}
}
}
