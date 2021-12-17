#include <toki/audio/AudioPlayer.h>
#include <toki/main/loadstate/LoadStateAudioPlayer.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

void LoadStateAudioPlayer::doLoadStep()
{
	if (m_initiatedPlayerCreation)
	{
		return;
	}
	
	m_initiatedPlayerCreation = true;
	
	if (audio::AudioPlayer::needsCreateOnMainThread())
	{
		//TT_Printf("LoadStateAudioPlayer::doLoadStep: Telling main thread to create AudioPlayer.\n");
		audio::AudioPlayer::triggerCreateOnMainThreadNow();
	}
	else
	{
		// Can simply create the audio player on the loading thread
		audio::AudioPlayer::createInstance();
	}
}


bool LoadStateAudioPlayer::isDone() const
{
	// This load state is done once we have an AudioPlayer
	return audio::AudioPlayer::hasInstance();
}

// Namespace end
}
}
}
