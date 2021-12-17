#include <tt/audio/codec/ogg/OggDecoder.h>
#include <tt/audio/player/OggPlayer.h>
#include <tt/code/helpers.h>
#include <tt/fs/fs.h>
#include <tt/math/math.h>
#include <tt/snd/snd.h>
#include <tt/snd/Stream.h>


namespace tt {
namespace audio {
namespace player {

// -----------------------------------------------------------------------------
// Public functions

OggPlayer::OggPlayer(fs::identifier p_fs, snd::identifier p_snd)
:
MusicPlayer(),
m_song(),
m_playing(false),
m_looping(false),
m_paused(false),
m_currentSongIsPreloaded(false),
m_streamHasStarted(false),
m_fs(p_fs),
m_snd(p_snd),
m_stream(),
m_decoder(0),
m_channelCount(0),
m_framerate(0)
{
}


OggPlayer::~OggPlayer()
{
	if (m_stream != 0)
	{
		m_stream->stop();
	}
	code::helpers::safeDelete(m_decoder);
}


// Playback functions

bool OggPlayer::play(const std::string& p_song, bool p_looping)
{
	if (m_playing && stop() == false)
	{
		return false;
	}
	
	if (preload(p_song) == false)
	{
		return false;
	}
	
	m_looping = p_looping;
	
	TT_NULL_ASSERT(m_stream);  // preload() should have created the stream
	m_streamHasStarted = false;
	m_stream->play();
	m_stream->setVolumeRatio(getPlaybackVolume());
	
	m_playing = true;
	m_currentSongIsPreloaded = false; // no longer preloaded, because we're consuming audio data now
	
	return true;
}


bool OggPlayer::stop()
{
	if (m_stream != 0)
	{
		if (m_stream->isPlaying())
		{
			m_stream->stop();
		}
		m_stream.reset();
	}
	
	m_paused                 = false;
	m_playing                = false;
	m_currentSongIsPreloaded = false;
	m_streamHasStarted       = false;
	
	return true;
}


bool OggPlayer::pause()
{
	if (m_song.empty() || m_stream == 0)
	{
		return false;
	}
	if (m_paused)
	{
		return true;
	}
	
	m_paused = true;
	
	if (m_playing)
	{
		m_stream->pause();
	}
	
	return true;
}


bool OggPlayer::resume()
{
	if (m_song.empty() || m_stream == 0)
	{
		return false;
	}
	if (m_paused == false)
	{
		return true;
	}
	
	m_paused = false;
	
	if (m_playing)
	{
		m_stream->resume();
	}
	
	return true;
}


void OggPlayer::update()
{
	if (m_playing && m_stream != 0)
	{
		m_stream->update();
		
		const bool streamIsPlaying = m_stream->isPlaying();
		if (m_streamHasStarted == false && streamIsPlaying)
		{
			m_streamHasStarted = true;
		}
		else if (m_streamHasStarted && streamIsPlaying == false)
		{
			m_playing = false;
		}
	}
}


// Status functions

const std::string& OggPlayer::getCurrentSong() const
{
	return m_song;
}


bool OggPlayer::isPlaying() const
{
	return m_playing;
}


bool OggPlayer::isPaused() const
{
	return m_paused;
}


bool OggPlayer::isLooping() const
{
	return m_looping;
}


bool OggPlayer::preload(const std::string& p_song)
{
	if (p_song != m_song)
	{
		code::helpers::safeDelete(m_decoder);
		m_currentSongIsPreloaded = false;
		
		if (fs::fileExists(p_song, m_fs) == false)
		{
			TT_PANIC("TTIM file '%s' does not exist. Cannot load it.", p_song.c_str());
			return false;
		}
		m_decoder = new codec::ogg::OggDecoder(p_song, m_fs);
	}
	
	if (m_currentSongIsPreloaded)
	{
		// No need to preload this song again: this was already done
		return true;
	}
	// Kill old stream
	m_stream.reset();
	
	m_framerate    = m_decoder->getSampleRate();
	m_channelCount = m_decoder->getChannelCount();
	m_decoder->rewind();
	m_song   = p_song;
	m_stream = snd::openStream(this, m_snd);
	
	m_currentSongIsPreloaded = true;
	
	return true;
}


bool OggPlayer::unload(const std::string& p_song)
{
	if (m_song != p_song)
	{
		// Song already unloaded (or different song requested for unload)
		return true;
	}
	
	if (stop() == false)
	{
		return false;
	}
	
	code::helpers::safeDelete(m_decoder);
	return true;
}


// StreamSource functions

snd::size_type OggPlayer::fillBuffer(snd::size_type  p_frames,
                                     snd::size_type  p_channels,
                                     void**          p_buffer,
                                     snd::size_type* /*p_notifySourceWhenPlaybackReachesThisFrame_OUT*/)
{
	TT_NULL_ASSERT(p_buffer);
	for (snd::size_type i = 0; i < p_channels; ++i)
	{
		TT_NULL_ASSERT(p_buffer[i]);
	}
	
	if (m_decoder == 0)
	{
		return 0;
	}
	
	snd::size_type frames = m_decoder->decode(codec::SampleType_Signed16, p_buffer, p_channels, p_frames);
	
	// The requested number of frames weren't decoded: reached end of stream?
	while (frames != p_frames)
	{
		if (m_looping)
		{
			if (m_decoder->rewind() == false)
			{
				TT_PANIC("Rewinding OggDecoder failed!");
				break;
			}
		}
		else
		{
			break;
		}
		
		frames += m_decoder->decode(codec::SampleType_Signed16, p_buffer, p_channels, p_frames - frames, frames);
	}
	
	return frames;
}


snd::size_type OggPlayer::fillBufferInterleaved(snd::size_type  p_frames,
                                                snd::size_type  p_channels,
                                                void*           p_buffer,
                                                snd::size_type* /*p_notifySourceWhenPlaybackReachesThisFrame_OUT*/)
{
	TT_NULL_ASSERT(p_buffer);
	
	if (m_decoder == 0)
	{
		return 0;
	}
	
	snd::size_type frames = m_decoder->decodeInterleaved(codec::SampleType_Signed16, p_buffer, p_channels, p_frames);
	
	// The requested number of frames weren't decoded: reached end of stream?
	while (frames != p_frames)
	{
		if (m_looping)
		{
			if (m_decoder->rewind() == false)
			{
				TT_PANIC("Rewinding OggDecoder failed!");
				break;
			}
		}
		else
		{
			break;
		}
		
		frames += m_decoder->decodeInterleaved(codec::SampleType_Signed16, p_buffer, p_channels, p_frames - frames, frames);
	}
	
	return frames;
}


snd::size_type OggPlayer::getSampleSize() const
{
	return 16;
}


snd::size_type OggPlayer::getBufferSize() const
{
	return m_framerate; // 1 second buffer
}


snd::size_type OggPlayer::getFramerate() const
{
	return m_framerate;
}


snd::size_type OggPlayer::getChannelCount() const
{
	return m_channelCount;
}


// -----------------------------------------------------------------------------
// Private functions

void OggPlayer::updateVolume(real p_volume)
{
	if (m_stream != 0)
	{
		m_stream->setVolumeRatio(p_volume);
	}
}


// namespace end
}
}
}
