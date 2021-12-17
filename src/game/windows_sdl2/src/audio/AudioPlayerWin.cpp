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


// XACT PLayer with OpenAL backend
tt::audio::player::TTXactPlayer* initOpenAL()
{
		tt::audio::player::TTXactPlayer* xact = new tt::audio::player::TTXactPlayer;
		if (xact->init("audio/rewind.bxap") == false)
		{
			TT_PANIC("Unable to initialize TT XACT (OpenAL).");
			delete xact;
			return 0;
		}
		return xact;
}

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
	
	tt::audio::player::SoundPlayer* player = 0;
	
	if (toki::AppGlobal::isAudioInSilentMode() == false)
	{
		player = initOpenAL();
	}
	
	ms_instance = new AudioPlayer(player);
}

// Namespace end
}
}
