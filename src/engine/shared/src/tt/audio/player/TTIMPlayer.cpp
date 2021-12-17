#include <tt/audio/helpers.h>
#include <tt/audio/codec/ogg/OggDecoder.h>
#include <tt/audio/codec/ttadpcm/TTAdpcmDecoder.h>
#include <tt/audio/codec/wav/WavDecoder.h>
#include <tt/audio/player/MusicPlayerCallbackInterface.h>
#include <tt/audio/player/TTIMPlayer.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/fs/utils/utils.h>
#include <tt/math/math.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/snd.h>
#include <tt/snd/Stream.h>
#include <tt/str/common.h>
#include <tt/str/parse.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

TTIMPlayer::TTIMPlayer(fs::identifier p_fs, snd::identifier p_snd)
:
MusicPlayer(),
m_song(),
m_looping(false),
m_playing(false),
m_paused(false),
m_currentBlockIsPreloaded(false),
m_streamHasStarted(false),
m_fs(p_fs),
m_snd(p_snd),
m_stream(),
m_properties(),
m_labels(),
m_gotos(),
m_playlist(),
m_musicblocks(),
m_decoders(),
m_songVolume(1.0f),
m_decoder(0),
m_currentBlock(0),
m_channelCount(0),
m_framerate(0),
m_notifyClientAboutLoopNextUpdate(false)
{
}


TTIMPlayer::~TTIMPlayer()
{
	if (m_stream != 0 && m_stream->isPlaying())
	{
		m_stream->stop();
	}
	
	destroy();
}


// Playback functions

bool TTIMPlayer::play(const std::string& p_song, bool p_looping)
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
	if (m_stream == 0)
	{
		destroy();
		return false;
	}
	m_streamHasStarted = false;
	m_stream->play();
	m_stream->setVolumeRatio(getPlaybackVolume());
	
	m_playing = true;
	
	m_currentBlockIsPreloaded = false;   // no longer preloaded, because we're consuming audio data now
	
	return true;
}


bool TTIMPlayer::stop()
{
	if (m_stream != 0)
	{
		if (m_stream->isPlaying())
		{
			m_stream->stop();
		}
		m_stream.reset();
	}
	
	m_paused                          = false;
	m_playing                         = false;
	m_notifyClientAboutLoopNextUpdate = false;
	m_currentBlockIsPreloaded         = false;
	m_streamHasStarted                = false;
	m_currentBlock                    = 0;
	m_song.clear();
	
	return true;
}


bool TTIMPlayer::pause()
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


bool TTIMPlayer::resume()
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


void TTIMPlayer::update()
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
			//TT_Printf("TTIMPlayer::update: [%p] Stream %p playback finished.\n", this, m_stream.get());
			m_playing = false;
		}
	}
	
	if (m_notifyClientAboutLoopNextUpdate)
	{
		m_notifyClientAboutLoopNextUpdate = false;
		
		MusicPlayerCallbackInterfacePtr callbackInterface = getCallbackInterface();
		if (callbackInterface != 0)
		{
			callbackInterface->onMusicPlayerLooped();
		}
	}
}


// Status functions

const std::string& TTIMPlayer::getCurrentSong() const
{
	return m_song;
}


bool TTIMPlayer::isPlaying() const
{
	return m_playing;
}


bool TTIMPlayer::isPaused() const
{
	return m_paused;
}


bool TTIMPlayer::isLooping() const
{
	return m_looping;
}


bool TTIMPlayer::preload(const std::string& p_song)
{
	return preloadBlock(p_song, m_currentBlock);
}


bool TTIMPlayer::unload(const std::string& p_song)
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
	
	destroy();
	return true;
}


// StreamSource functions

snd::size_type TTIMPlayer::fillBuffer(snd::size_type  p_frames,
                                      snd::size_type  p_channels,
                                      void**          p_buffer,
                                      snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT)
{
	if (m_decoder == 0)
	{
		return 0;
	}
	
	snd::size_type frames = m_decoder->decode(codec::SampleType_Signed16,
	                                          p_buffer, p_channels, p_frames);
	
	while (frames != p_frames)
	{
		m_decoder->rewind();
		if (*p_notifySourceWhenPlaybackReachesThisFrame_OUT < 0)
		{
			*p_notifySourceWhenPlaybackReachesThisFrame_OUT = frames;
		}
		m_decoder = getNextDecoder();
		
		if (m_decoder == 0)
		{
			break;
		}
		
		frames += m_decoder->decode(codec::SampleType_Signed16,
		                            p_buffer, p_channels, p_frames - frames, frames);
	}
	return frames;
}


snd::size_type TTIMPlayer::fillBufferInterleaved(snd::size_type  p_frames,
                                                 snd::size_type  p_channels,
                                                 void*           p_buffer,
                                                 snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT)
{
	if (m_decoder == 0)
	{
		return 0;
	}
	
	snd::size_type frames = m_decoder->decodeInterleaved(codec::SampleType_Signed16,
	                                                     p_buffer, p_channels, p_frames);
	
	while (frames != p_frames)
	{
		m_decoder->rewind();
		if (*p_notifySourceWhenPlaybackReachesThisFrame_OUT < 0)
		{
			*p_notifySourceWhenPlaybackReachesThisFrame_OUT = frames;
		}
		m_decoder = getNextDecoder();
		
		if (m_decoder == 0)
		{
			break;
		}
		
		frames += m_decoder->decodeInterleaved(codec::SampleType_Signed16,
		                                       p_buffer, p_channels, p_frames - frames, frames);
	}
	return frames;
}


snd::size_type TTIMPlayer::getSampleSize() const
{
	return 16;
}


snd::size_type TTIMPlayer::getBufferSize() const
{
	return m_framerate; // 1 second buffer
}


snd::size_type TTIMPlayer::getFramerate() const
{
	return m_framerate;
}


snd::size_type TTIMPlayer::getChannelCount() const
{
	return m_channelCount;
}


void TTIMPlayer::onStreamReachedNotificationFrame()
{
	m_notifyClientAboutLoopNextUpdate = true;
}


bool TTIMPlayer::preloadBlock(const std::string& p_song, s32 p_block)
{
	if (p_song != m_song)
	{
		if (loadSong(p_song) == false)
		{
			TT_PANIC("Failed to load '%s'", p_song.c_str());
			return false;
		}
		
		m_currentBlock            = p_block;
		m_song                    = p_song;
		m_currentBlockIsPreloaded = false;
	}
	else if (m_currentBlock != p_block)
	{
		m_currentBlock            = p_block;
		m_currentBlockIsPreloaded = false;
	}
	
	if (m_currentBlock < 0 || m_currentBlock >= static_cast<s32>(m_playlist.size()))
	{
		TT_PANIC("Invalid block '%d' playlist size is '%d'", m_currentBlock, m_playlist.size());
		return false;
	}
	
	if (m_currentBlockIsPreloaded)
	{
		return true;
	}
	
	return preloadCurrentBlock();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TTIMPlayer::updateVolume(real p_volume)
{
	if (m_stream != 0)
	{
		m_stream->setVolumeRatio(p_volume);
	}
}


bool TTIMPlayer::preloadCurrentBlock()
{
	TT_ASSERT(m_currentBlockIsPreloaded == false);
	
	codec::Decoder* oldDecoder = m_decoder;
	if (oldDecoder != 0)
	{
		oldDecoder->rewind();
	}
	
	TT_ASSERT(m_currentBlock >= 0 && m_currentBlock < static_cast<s32>(m_playlist.size()));
	
	m_decoder = m_decoders[m_playlist[m_currentBlock]];
	TT_NULL_ASSERT(m_decoder);
	if (m_decoder != oldDecoder)
	{
		m_decoder->rewind();
	}
	
	if (m_stream != 0)
	{
		if (m_stream->isPlaying())
		{
			m_stream->stop();
		}
		m_stream.reset();
	}
	m_stream = snd::openStream(this, m_snd);
	
	m_currentBlockIsPreloaded = true;
	
	return true;
}


bool TTIMPlayer::loadSong(const std::string& p_song)
{
	// Clear old data
	destroy();
	
	if (fs::fileExists(p_song, m_fs) == false)
	{
		TT_PANIC("TTIM file '%s' does not exist. Cannot load it.", p_song.c_str());
		return false;
	}
	
	// Load file
	code::BufferPtr content = fs::getFileContent(p_song, m_fs);
	if (content == 0 || content->getSize() == 0)
	{
		TT_WARN("Unable to open '%s' or file is empty.", p_song.c_str());
		return false;
	}
	
	// Parse TTIM file
	const char* data  = reinterpret_cast<const char*>(content->getData());
	code::Buffer::size_type read  = content->getSize();
	code::Buffer::size_type index = 0;
	while (index < read)
	{
		code::Buffer::size_type start = index;
		
		// read until next newline
		while (index < read)
		{
			if (data[index] == '\n' || data[index] == '\r')
			{
				break;
			}
			++index;
		}
		
		std::string line(data + start, data + index);
		if (line.empty())
		{
			++index;
			continue;
		}
		
		// parse line
		if (line[0] == '#')
		{
			// comment, skip this
			continue;
		}
		
		if (str::startsWith(line, "goto "))
		{
			// goto found
			line = line.substr(5);
			if (m_playlist.empty())
			{
				TT_WARN("Found goto at song start (in %s), ignoring.", p_song.c_str());
			}
			else
			{
				if (m_gotos.find(m_playlist.size() - 1) == m_gotos.end())
				{
					m_gotos.insert(Gotos::value_type(m_playlist.size() - 1, line));
				}
				else
				{
					TT_WARN("Found two gotos after eachother (in %s), ignoring.", p_song.c_str());
				}
			}
		}
		else if (str::endsWith(line, ":"))
		{
			// label found
			line = line.substr(0, line.length() - 1);
			if (m_labels.find(line) != m_labels.end())
			{
				TT_WARN("Label '%s' found twice (in %s), ignoring.", line.c_str(), p_song.c_str());
			}
			else
			{
				m_labels.insert(Labels::value_type(line, m_playlist.size()));
			}
		}
		else if (str::endsWith(line, ";"))
		{
			// variable assignment found
			std::string::size_type pos = line.find('=');
			if (pos == std::string::npos)
			{
				TT_WARN("Found property assignment without assignment operator (=) (in %s), ignoring.", p_song.c_str());
			}
			else
			{
				std::string prop = line.substr(0, pos);
				std::string val = line.substr(pos + 1, line.length() - (pos + 2));
				if (m_properties.find(prop) != m_properties.end())
				{
					TT_WARN("Warning, overwriting property '%s' (in %s)", prop.c_str(), p_song.c_str());
				}
				m_properties[prop] = val;
			}
		}
		else
		{
			// music block found
			Musicblocks::iterator pos = std::find(m_musicblocks.begin(), m_musicblocks.end(), line);
			if (pos == m_musicblocks.end())
			{
				m_playlist.push_back(m_musicblocks.size());
				m_musicblocks.push_back(line);
			}
			else
			{
				// find index
				m_playlist.push_back(static_cast<Playlist::size_type>(pos - m_musicblocks.begin()));
			}
		}
		
		++index;
	}
	
	// clean up
	content.reset();
	
	if (m_playlist.empty())
	{
		TT_WARN("Playlist is empty (%s).", p_song.c_str());
		destroy();
		return false;
	}
	
	// check all gotos
	bool gotoValid = true;
	for (Gotos::iterator it = m_gotos.begin(); it != m_gotos.end(); ++it)
	{
		if (m_labels.find((*it).second) == m_labels.end())
		{
			TT_WARN("[%d] goto '%s': unknown label (in %s).",
			        (*it).first, (*it).second.c_str(), p_song.c_str());
			gotoValid = false;
		}
	}
	
	if (gotoValid == false)
	{
		TT_WARN("Encountered invalid goto(s) (in %s).", p_song.c_str());
		destroy();
		return false;
	}
	
	// load all musicblocks
	for (Musicblocks::iterator it = m_musicblocks.begin(); it != m_musicblocks.end(); ++it)
	{
		codec::Decoder* decoder = 0;
		
		// get file extension
		std::string path = m_properties["root"] + (*it);
		if (fs::fileExists(path, m_fs) == false)
		{
			TT_PANIC("Audio file '%s' does not exist.\nReferenced from TTIM file '%s'.",
			         path.c_str(), p_song.c_str());
			destroy();
			return false;
		}
		
		std::string extension = fs::utils::getExtension(path, m_fs);
		if (extension == "ogg")
		{
			decoder = new codec::ogg::OggDecoder(path, m_fs);
		}
		else if (extension == "ttadpcm")
		{
			decoder = new codec::ttadpcm::TTAdpcmDecoder(path, m_fs);
		}
		else if (extension == "wav")
		{
			decoder = new codec::wav::WavDecoder(path, m_fs);
		}
		else
		{
			TT_PANIC("Unknown file type for '%s' in '%s'.", path.c_str(), p_song.c_str());
		}
		
		if (decoder == 0)
		{
			TT_PANIC("Unable to create decoder for '%s' in '%s'", path.c_str(), p_song.c_str());
			destroy();
			return false;
		}
		
		if (it == m_musicblocks.begin())
		{
			m_channelCount = decoder->getChannelCount();
			m_framerate    = decoder->getSampleRate();
		}
		else
		{
			if (decoder->getChannelCount() != m_channelCount)
			{
				TT_PANIC("Decoder for '%s' has a non matching channel count (%d, expected %d).",
				         path.c_str(), decoder->getChannelCount(), m_channelCount);
			}
			if (decoder->getSampleRate() != m_framerate)
			{
				TT_PANIC("Decoder for '%s' has a non matching framerate (%d, expected %d).",
				         path.c_str(), decoder->getSampleRate(), m_framerate);
			}
		}
		
		m_decoders.push_back(decoder);
	}
	
	Properties::iterator it = m_properties.find("volume");
	if (it != m_properties.end())
	{
		TT_ERR_CREATE("TTIMPlayer volume parsing.");
		real volume = str::parseReal((*it).second, &errStatus);
		if (errStatus.hasError())
		{
			TT_WARN("Unable to parse volume '%s' (in '%s').", (*it).second.c_str(), p_song.c_str());
		}
		else
		{
			m_songVolume = helpers::dBToRatio(volume);
		}
	}
	
	m_currentBlock = 0;
	m_decoder = m_decoders[m_playlist[m_currentBlock]];
	
	return true;
}


void TTIMPlayer::destroy()
{
	m_stream.reset();
	
	code::helpers::freePointerContainer(m_decoders);
	m_labels.clear();
	m_gotos.clear();
	m_playlist.clear();
	m_musicblocks.clear();
	m_properties.clear();
	m_decoder = 0;
	m_currentBlock = 0;
	
	m_songVolume = 1.0f;
	m_channelCount = 0;
	m_framerate = 0;
	
	m_song.clear();
	m_currentBlockIsPreloaded = false;
}


codec::Decoder* TTIMPlayer::getNextDecoder()
{
	Gotos::iterator it = m_gotos.find(m_currentBlock);
	
	if (it == m_gotos.end())
	{
		// no goto, get next block
		++m_currentBlock;
		if (m_currentBlock >= static_cast<s32>(m_playlist.size()))
		{
			// end of song
			m_currentBlock = 0;
			return 0;
		}
		return m_decoders[m_playlist[m_currentBlock]];
	}
	else
	{
		// find label, get value, assign to currentblock
		m_currentBlock = static_cast<s32>((*m_labels.find((*it).second)).second);
		if (m_currentBlock >= static_cast<s32>(m_playlist.size()))
		{
			// label may be positioned at end of song
			m_currentBlock = 0;
			return 0;
		}
		return m_decoders[m_playlist[m_currentBlock]];
	}
}


codec::Decoder* TTIMPlayer::findDecoder(const std::string& p_label)
{
	Labels::iterator it = m_labels.find(p_label);
	if (it == m_labels.end())
	{
		TT_WARN("Label '%s' not found.", p_label.c_str());
		return 0;
	}
	
	m_currentBlock = static_cast<s32>((*it).second);
	if (m_currentBlock >= static_cast<s32>(m_playlist.size()))
	{
		// label may be positioned at end of song
		return 0;
	}
	
	return m_decoders[m_playlist[m_currentBlock]];
}

// Namespace end
}
}
}
