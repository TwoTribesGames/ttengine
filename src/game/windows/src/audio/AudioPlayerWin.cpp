#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/audio/player/TTXactPlayer.h>
#include <tt/audio/player/Xact3Player.h>
#include <tt/platform/tt_error.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/AppGlobal.h>

#include "constants_win.h"


namespace toki {
namespace audio {

//--------------------------------------------------------------------------------------------------
// Public member functions


#ifdef USE_OPENAL_BACKEND
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
#endif


// XACT 3 Player with XAudio2 backend
tt::audio::player::Xact3Player* initXact()
{
	tt::audio::player::Xact3Player* xact = new tt::audio::player::Xact3Player(false);
	if (xact->init("audio/rewind.xgs")                   == false ||
	    xact->loadSoundBank("audio/Effects.xsb")         == false ||
	    xact->loadWaveBank ("audio/Effects.xwb", true)   == false ||
	    xact->loadSoundBank("audio/VoiceOver.xsb")       == false ||
	    xact->loadWaveBank ("audio/VoiceOver.xwb", true) == false ||
	    xact->loadSoundBank("audio/Ambient.xsb")         == false ||
	    xact->loadWaveBank ("audio/Ambient.xwb", true)   == false ||
	    xact->loadSoundBank("audio/Music.xsb")           == false ||
	    xact->loadWaveBank ("audio/Music.xwb", true)     == false)
	{
		TT_PANIC("Unable to initialize XACT.");
		delete xact;
		return 0;
	}
	
	return xact;
}



bool AudioPlayer::needsCreateOnMainThread()
{
#ifdef USE_OPENAL_BACKEND
	return false;
#else
	return true;
#endif
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
#ifdef USE_OPENAL_BACKEND
		player = initOpenAL();
#else
		player = initXact();
#endif
	}
	
	ms_instance = new AudioPlayer(player);
}

// Namespace end
}
}
