#include <tt/app/ComHelper.h>
#include <tt/audio/player/WMPPlayer.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

WMPPlayer* WMPPlayer::create()
{
	// Initialize COM, since we're using Windows Media Player via COM
	app::ComHelper::initCom();
	
	// Create a IWMPPlayer object and retrieve the required interfaces
	CComPtr<IWMPPlayer>   wmpPlayer;
	CComPtr<IWMPCore>     wmpCore;
	CComPtr<IWMPCore3>    wmpCore3;
	CComPtr<IWMPControls> wmpControls;
	CComPtr<IWMPPlaylist> wmpPlaylist;
	CComPtr<IWMPSettings> wmpSettings;
	
	HRESULT hr = wmpPlayer.CoCreateInstance(__uuidof(WindowsMediaPlayer), 0, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		TT_PANIC("Creating IWMPPlayer instance failed.");
		return 0;
	}
	
	// Output the WMP version information for debugging purposes
	{
		CComBSTR versionInfo;
		hr = wmpPlayer->get_versionInfo(&versionInfo);
		if (SUCCEEDED(hr))
		{
			CW2A ansiVersionInfo(versionInfo);
			TT_Printf("WMPPlayer::create: Using Windows Media Player version %s.\n",
			          (LPSTR)ansiVersionInfo);
		}
	}
	
	// Initialize the various player interfaces
	hr = wmpPlayer->QueryInterface(&wmpCore);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving IWMPCore component from IWMPPlayer failed.");
		return 0;
	}
	
	hr = wmpCore->QueryInterface(&wmpCore3);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving IWMPCore3 component from IWMPCore failed.");
		return 0;
	}
	
	hr = wmpPlayer->get_settings(&wmpSettings);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving settings (IWMPSettings) from IWMPPlayer failed.");
		return 0;
	}
	
	hr = wmpPlayer->get_controls(&wmpControls);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving controls (IWMPControls) from IWMPPlayer failed.");
		return 0;
	}
	
	hr = wmpPlayer->get_currentPlaylist(&wmpPlaylist);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving current playlist (IWMPPlaylist) from IWMPPlayer failed.");
		return 0;
	}
	
	// All required interfaces have been retrieved; create the player
	return new WMPPlayer(wmpPlayer,
	                     wmpCore,
	                     wmpCore3,
	                     wmpControls,
	                     wmpPlaylist,
	                     wmpSettings);
}


WMPPlayer::~WMPPlayer()
{
	// Release all COM objects/interfaces
	m_wmpPlayer.Release();
	m_wmpCore.Release();
	m_wmpCore3.Release();
	m_wmpControls.Release();
	m_wmpPlaylist.Release();
	m_wmpSettings.Release();
	
	// Uninitialize COM
	app::ComHelper::uninitCom();
}


bool WMPPlayer::play(const std::string& p_song, bool p_looping)
{
	m_isPlayingJingle = false;
	return playImpl(p_song, p_looping, 0.0);
}


bool WMPPlayer::stop()
{
	if (FAILED(m_wmpControls->stop()))
	{
		TT_PANIC("Stopping WMP playback failed.");
		return false;
	}
	
	m_currentSong.clear();
	m_currentSongLooping = false;
	m_isPlayingJingle    = false;
	m_songResumePosition = 0.0;
	m_songResumeName.clear();
	m_songResumeLooping  = false;
	
	return true;
}


bool WMPPlayer::pause()
{
	return SUCCEEDED(m_wmpControls->pause());
}


bool WMPPlayer::resume()
{
	// FIXME: Only if paused?
	return SUCCEEDED(m_wmpControls->play());
}


void WMPPlayer::update()
{
	// FIXME: This polling implementation needs to be replaced with COM event handling
	if (m_isPlayingJingle)
	{
		// Retrieve the current play state to see if the jingle is finished playing
		WMPPlayState playState = wmppsUndefined;
		HRESULT      hr        = m_wmpPlayer->get_playState(&playState);
		if (SUCCEEDED(hr) && (playState == wmppsMediaEnded || playState == wmppsStopped))
		{
			// Jingle ended; resume the music that was previously playing
			//TT_Printf("WMPPlayer::update: Jingle finished playing. Resuming music.\n");
			const std::string resumeSong(m_songResumeName);
			const bool        resumeLooping = m_songResumeLooping;
			const double      resumePos     = m_songResumePosition;
			m_songResumeName.clear();
			m_songResumeLooping  = false;
			m_songResumePosition = 0.0;
			if (resumeSong.empty() == false)
			{
				playImpl(resumeSong, resumeLooping, resumePos);
			}
			m_isPlayingJingle    = false;
		}
	}
}


bool WMPPlayer::isPlaying() const
{
	// Not playing if there is no song
	if (m_currentSong.empty())
	{
		return false;
	}
	
	// Retrieve the current play state
	WMPPlayState playState = wmppsUndefined;
	HRESULT      hr        = m_wmpPlayer->get_playState(&playState);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving WMP playback state failed.");
		return false;
	}
	
	// Song is considered to be playing if it is either playing or paused
	return playState == wmppsPlaying || playState == wmppsPaused;
}


bool WMPPlayer::isPaused() const
{
	// Not paused if there is no song
	if (m_currentSong.empty())
	{
		return false;
	}
	
	// Retrieve the current play state
	WMPPlayState playState = wmppsUndefined;
	HRESULT      hr        = m_wmpPlayer->get_playState(&playState);
	if (FAILED(hr))
	{
		TT_PANIC("Retrieving WMP playback state failed.");
		return false;
	}
	
	// Song is considered to be paused only if the play state is paused
	return playState == wmppsPaused;
}


bool WMPPlayer::isLooping() const
{
	return m_currentSongLooping;
}


bool WMPPlayer::preload(const std::string& p_song)
{
	TT_PANIC("Music preloading not supported in WMPPlayer.");
	(void)p_song;
	return false;
}


bool WMPPlayer::unload(const std::string& p_song)
{
	TT_PANIC("Music preloading not supported in WMPPlayer.");
	(void)p_song;
	return false;
}


bool WMPPlayer::isLoaded(const std::string& p_song)
{
	TT_PANIC("Music preloading not supported in WMPPlayer.");
	(void)p_song;
	return false;
}


bool WMPPlayer::playJingle(const std::string& p_jingle)
{
	if (m_isPlayingJingle == false && m_currentSong.empty() == false)
	{
		// Remember the current position of the song so we can restore it
		HRESULT hr = m_wmpControls->get_currentPosition(&m_songResumePosition);
		if (FAILED(hr))
		{
			TT_PANIC("Retrieving playback position of currently playing song failed.");
			m_songResumePosition = 0.0;
		}
		m_songResumeName    = m_currentSong;
		m_songResumeLooping = m_currentSongLooping;
		
	}
	m_isPlayingJingle = true;
	
	return playImpl(p_jingle, false, 0.0);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

WMPPlayer::WMPPlayer(const CComPtr<IWMPPlayer>&   p_wmpPlayer,
                     const CComPtr<IWMPCore>&     p_wmpCore,
                     const CComPtr<IWMPCore3>&    p_wmpCore3,
                     const CComPtr<IWMPControls>& p_wmpControls,
                     const CComPtr<IWMPPlaylist>& p_wmpPlaylist,
                     const CComPtr<IWMPSettings>& p_wmpSettings)
:
MusicJinglePlayer(),
m_currentSongLooping(false),
m_wmpPlayer(p_wmpPlayer),
m_wmpCore(p_wmpCore),
m_wmpCore3(p_wmpCore3),
m_wmpControls(p_wmpControls),
m_wmpPlaylist(p_wmpPlaylist),
m_wmpSettings(p_wmpSettings),
m_isPlayingJingle(false),
m_songResumePosition(0.0),
m_songResumeLooping(false)
{
}


void WMPPlayer::updateVolume(real p_volume)
{
	// WMP volume is in range 0 - 100, TT volume is normalized (0 - 1)
	m_wmpSettings->put_volume(static_cast<long>(p_volume * 100.0f));
}


bool WMPPlayer::playImpl(const std::string& p_song, bool p_looping, double p_resumePosition)
{
	if (fs::fileExists(p_song) == false)
	{
		TT_PANIC("File '%s' does not exist; cannot play music.", p_song.c_str());
		return false;
	}
	
	HRESULT hr = S_OK;
	
	// Clear the current playlist
	hr = m_wmpPlaylist->clear();
	if (FAILED(hr))
	{
		TT_PANIC("Clearing WMP playlist failed.");
		return false;
	}
	
	// Create a new media item
	CComBSTR pathToOpen(p_song.c_str());
	CComPtr<IWMPMedia> wmpMedia;
	
	hr = m_wmpCore3->newMedia(pathToOpen, &wmpMedia);
	if (FAILED(hr))
	{
		TT_PANIC("Creating new media item for MP3 file failed.");
		return false;
	}
	
	// Add the media item to the playlist
	hr = m_wmpPlaylist->appendItem(wmpMedia);
	if (FAILED(hr))
	{
		TT_PANIC("Appending new media item to playlist failed.");
		return false;
	}
	
	// Set the looping mode (also always disable shuffle)
	hr = m_wmpSettings->setMode(CComBSTR(L"loop"), p_looping ? VARIANT_TRUE : VARIANT_FALSE);
	if (FAILED(hr))
	{
		TT_PANIC("Setting looping mode in WMP playback settings failed.");
		return false;
	}
	
	hr = m_wmpSettings->setMode(CComBSTR(L"shuffle"), VARIANT_FALSE);
	if (FAILED(hr))
	{
		TT_PANIC("Disabling shuffle in WMP playback settings failed.");
		return false;
	}
	
	// Play the music
	hr = m_wmpControls->play();
	if (FAILED(hr))
	{
		TT_PANIC("Playing playlist failed.");
		return false;
	}
	
	hr = m_wmpControls->put_currentPosition(p_resumePosition);
	TT_ASSERTMSG(SUCCEEDED(hr),
	             "Setting playback position for song '%s' to %.3lf failed.",
	             p_song.c_str(), p_resumePosition);
	
	m_currentSong        = p_song;
	m_currentSongLooping = p_looping;
	
	return true;
}

// Namespace end
}
}
}
