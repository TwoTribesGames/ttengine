#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/audio/player/TTXactPlayer.h>
#include <tt/platform/tt_error.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace audio {

//--------------------------------------------------------------------------------------------------
// Public member functions

bool AudioPlayer::needsCreateOnMainThread()
{
	return false;
}


// NOTE: This implementation of AudioPlayer::createInstance
//       creates the TT libs version of the audio player.
void AudioPlayer::createInstance()
{
	if (ms_instance != 0)
	{
		TT_PANIC("Instance already exists!");
		return;
	}
	
	tt::audio::player::TTXactPlayer* xact = 0;
	
	if (toki::AppGlobal::isAudioInSilentMode() == false)
	{
		xact = new tt::audio::player::TTXactPlayer;
		if (xact->init("audio/rewind.bxap") == false)
		{
			TT_PANIC("Unable to initialize XACT.");
			delete xact;
			xact = 0;
		}
	}
	
	ms_instance = new AudioPlayer(xact);
}

// Namespace end
}
}
