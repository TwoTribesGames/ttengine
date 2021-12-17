#include <tt/audio/chibi/TTFileIO.h>
#include <tt/audio/chibi/XMUtil.h>
#include <tt/audio/player/XMMusicPlayer.h>
#include <tt/audio/chibi/StreamMixer.h>
#include <tt/code/helpers.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>


namespace tt {
namespace audio {
namespace player {

namespace details {
class MemoryManager : public chibi::XMMemoryManager
{
public:
	virtual ~MemoryManager() {}
	
	inline virtual void* alloc(u32 p_size, AllocType p_allocType)
	{
#if defined (TT_PLATFORM_OSX)
		(void)p_allocType;
		u32 alignment = 32;
		s32 type = 0;
#elif defined (TT_PLATFORM_WIN)
		(void)p_allocType;
		u32 alignment = 32;
		s32 type = 0;
#else
		(void)p_allocType;
		u32 alignment = 32;
		s32 type = 0;
#endif
		return mem::alloc(p_size, alignment, 0, type);
	}
	
	inline virtual void free(void* p_mem, AllocType /*p_freeType*/)
	{
		mem::free(p_mem);
	}
	
	inline virtual void zeroMem(void* p_mem, u32 p_size)
	{
		mem::zero8(p_mem, static_cast<mem::size_type>(p_size));
	}
};
}


// -----------------------------------------------------------------------------
// Public functions

XMMusicPlayer::XMMusicPlayer(fs::identifier     p_fs,
                             snd::identifier    p_snd,
                             const std::string& p_instruments,
                             int                p_samplerate,
                             bool               p_hardware)
:
m_paused(false),
m_looping(false),
m_currentLoaded(false),
m_currentSong(),
m_memMgr(new details::MemoryManager),
m_player(0),
m_mixer(0),
m_song(0),
m_loader(0),
m_fileIO(0),
m_patterns(),
m_isPlayingJingle(false),
m_songResumePosition(0),
m_songResumeName(),
m_songResumeLooping(false)
{
	chibi::XMUtil::setMemoryManager(m_memMgr);
	m_player = new chibi::XMPlayer;
	
#if defined (TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
	(void)p_hardware;
	m_mixer = new chibi::StreamMixer(p_samplerate, p_samplerate, p_snd);
#elif defined (TT_PLATFORM_WIN)
	(void)p_hardware;
	m_mixer = new chibi::StreamMixer(p_samplerate, p_samplerate, p_snd);
#else
	(void)p_hardware;
	(void)p_samplerate;
	(void)p_snd;
	m_mixer = 0;
#endif
	m_player->setMixer(m_mixer);
	
	m_song = new chibi::XMSong;
	m_loader = new chibi::XMLoader;
	m_loader->setPlayer(m_player);
	m_fileIO = new chibi::TTFileIO(p_fs);
	m_loader->setFileIO(m_fileIO);
	
	chibi::XMLoader::Error result = m_loader->openCustomInstruments(p_instruments.c_str(), m_song);
	if (result != chibi::XMLoader::Error_Ok)
	{
		TT_PANIC("Loading instrument bank '%s' failed: %s.",
			p_instruments.c_str(), chibi::XMLoader::getErrorDescription(result));
	}
	
	m_player->setSong(m_song);
}


XMMusicPlayer::~XMMusicPlayer()
{
	// stop playing
	if (isPlaying())
	{
		stop();
	}
	
	delete m_song;
	code::helpers::freePairSecondContainer(m_patterns);
	delete m_player;
	delete m_mixer;
	delete m_loader;
	delete m_fileIO;
	chibi::XMUtil::setMemoryManager(0);
	delete m_memMgr;
}


// Playback functions

bool XMMusicPlayer::play(const std::string& p_song, bool p_looping)
{
	m_isPlayingJingle = false;
	return playImpl(p_song, p_looping, getOrder(p_song));
}


bool XMMusicPlayer::stop()
{
	m_player->stop();
	
	m_paused = false;
	std::string current = m_currentSong;
	m_currentSong.clear();
	
	if (m_currentLoaded)
	{
		unload(current);
	}
	
	return true;
}


bool XMMusicPlayer::pause()
{
	if (m_currentSong.empty())
	{
		return false;
	}
	if (m_paused)
	{
		return true;
	}
	
	m_paused = true;
	m_mixer->pause();
	
	return true;
}


bool XMMusicPlayer::resume()
{
	if (m_currentSong.empty())
	{
		return false;
	}
	if (m_paused == false)
	{
		return true;
	}
	
	m_paused = false;
	m_mixer->resume();
	
	return true;
}


bool XMMusicPlayer::playJingle(const std::string& p_jingle)
{
	if (m_isPlayingJingle == false)
	{
		// Remember the current position of the song so we can restore it.
		m_songResumePosition   = m_player->getCurrentOrder();
		m_songResumeName       = m_currentSong;
		m_songResumeLooping    = m_looping;
		
		m_isPlayingJingle    = true;
	}
	
	return playImpl(p_jingle, false, getOrder(p_jingle));
}


void XMMusicPlayer::update()
{
	m_mixer->update();
	if (m_isPlayingJingle && m_player->hasSongReachedEnd())
	{
		if (m_songResumeName.empty() == false)
		{
			playImpl(m_songResumeName, m_songResumeLooping, m_songResumePosition);
			m_songResumeName.clear();
		}
		m_isPlayingJingle = false;
	}
}


// Status functions

const std::string& XMMusicPlayer::getCurrentSong() const
{
	return m_currentSong;
}


bool XMMusicPlayer::isPlaying() const
{
	return m_currentSong.empty() == false;
}


bool XMMusicPlayer::isPaused() const
{
	return m_paused;
}


bool XMMusicPlayer::isLooping() const
{
	return m_looping;
}


bool XMMusicPlayer::preload(const std::string& p_song)
{
	if (isLoaded(p_song))
	{
		return true;
	}
	
	std::string file = getFilename(p_song);
	
	chibi::XMSong* song = new chibi::XMSong;
	chibi::XMLoader::Error result = m_loader->openCustomMusic(file.c_str(), song);
	if (result != chibi::XMLoader::Error_Ok)
	{
		TT_PANIC("Loading pattern data '%s' failed: %s.",
			file.c_str(), chibi::XMLoader::getErrorDescription(result));
		delete song;
		return false;
	}
	m_patterns[file] = song;
	return true;
}


bool XMMusicPlayer::unload(const std::string& p_song)
{
	if (isLoaded(p_song) == false)
	{
		return true;
	}
	
	std::string file = getFilename(p_song);
	
	if (m_currentSong == file)
	{
		return false;
	}
	
	delete m_patterns[file];
	m_patterns.erase(file);
	return true;
}


bool XMMusicPlayer::isLoaded(const std::string& p_song)
{
	return m_patterns.find(getFilename(p_song)) != m_patterns.end();
}


// -----------------------------------------------------------------------------
// Private functions

void XMMusicPlayer::updateVolume(real p_volume)
{
	m_mixer->setVolume(p_volume);
}


std::string XMMusicPlayer::getFilename(const std::string& p_song) const
{
	std::string::size_type pos = p_song.rfind(':');
	if (pos == std::string::npos || (pos == 1 && p_song.find('\\') == 2))
	{
		// no : found or : is second character (as in C:\foo\bar.xmp)
		return p_song;
	}
	return p_song.substr(0, pos);
}


u8 XMMusicPlayer::getOrder(const std::string& p_song) const
{
	std::string::size_type pos = p_song.rfind(':');
	if (pos == std::string::npos || (pos == 1 && p_song.find('\\') == 2))
	{
		// no : found or : is second character (as in C:\foo\bar.xmp)
		return 0;
	}
	std::string order = p_song.substr(pos + 1);
	if (order.empty())
	{
		return 0;
	}
	return str::parseU8(order, 0);
}


bool XMMusicPlayer::playImpl(const std::string& p_song, bool p_looping, u8 p_resumePosition)
{
	if (stop() == false)
	{
		return false;
	}
	
	if (isLoaded(p_song) == false)
	{
		if (preload(p_song) == false)
		{
			TT_PANIC("Failed to load '%s'", p_song.c_str());
			return false;
		}
		m_currentLoaded = true;
	}
	else
	{
		m_currentLoaded = false;
	}
	
	m_currentSong = getFilename(p_song);
	m_looping     = p_looping;
	m_paused      = false;
	
	m_song->sharePatternData(m_patterns[m_currentSong]);
	m_player->play(m_looping, p_resumePosition);
	
	return true;
}


// namespace end
}
}
}
