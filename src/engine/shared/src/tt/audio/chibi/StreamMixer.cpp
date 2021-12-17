#include <tt/audio/chibi/StreamMixer.h>
#include <tt/code/helpers.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/snd.h>
#include <tt/snd/Stream.h>


namespace tt {
namespace audio {
namespace chibi {


StreamMixer::StreamMixer(int p_sampleRate, int p_bufferSize, snd::identifier p_snd)
:
XMSoftwareMixer(static_cast<u32>(p_sampleRate), 32),
m_snd(p_snd),
m_stream(),
m_channelCount(2),
m_framerate(p_sampleRate),
m_bufferSize(p_bufferSize),
m_volume(1.0f),
m_xmBuffer(new s32[p_bufferSize * /*m_channelCount*/2])
{
}


StreamMixer::~StreamMixer()
{
	if (m_stream != 0)
	{
		m_stream->stop();
	}
	m_stream.reset();
	
	delete[] m_xmBuffer;
}


void StreamMixer::play()
{
	stop();
	
	m_stream = snd::openStream(this, m_snd);
	m_stream->play();
	m_stream->setVolumeRatio(m_volume);
}


void StreamMixer::stop()
{
	if (m_stream == 0)
	{
		return;
	}
	
	m_stream->stop();
	m_stream.reset();
}


void StreamMixer::pause()
{
	if (m_stream != 0 && 
	    m_stream->isPaused() == false)
	{
		m_stream->pause();
	}
}


void StreamMixer::resume()
{
	if (m_stream != 0 && 
	    m_stream->isPaused())
	{
		m_stream->resume();
	}
}


bool StreamMixer::update()
{
	if (m_stream != 0)
	{
		if (m_stream->update() == false)
		{
			// FIXME: A false return value could indicate an error, 
			//        but it could also indicate the end of a stream.
			pause();
			return false;
		}
		return true;
	}
	return false;
}


void StreamMixer::setVolume(real p_volume)
{
	if (m_stream != 0)
	{
		m_stream->setVolumeRatio(p_volume);
	}
	
	m_volume = p_volume;
}


// StreamSource functions

snd::size_type StreamMixer::fillBuffer(snd::size_type  p_frames,
                                       snd::size_type  p_channels,
                                       void**          p_buffer,
                                       snd::size_type* /*p_notifySourceWhenPlaybackReachesThisFrame_OUT*/)
{
	snd::size_type frames = 0;
	while (frames < p_frames)
	{
		// clear xm buffer
		mem::zero32(m_xmBuffer, static_cast<mem::size_type>(m_bufferSize * m_channelCount * sizeof(s32)));
		
		// render xm to buffer
		snd::size_type todo = p_frames - frames;
		snd::size_type result = static_cast<snd::size_type>(mixToBuffer(m_xmBuffer, static_cast<u32>(todo)));
		
		s32* scratch = m_xmBuffer;
		
		// de-interleave
		for (snd::size_type frame = 0; frame < result; ++frame)
		{
			for (snd::size_type channel = 0; channel < m_channelCount; ++channel)
			{
				// scale down
				if (channel < p_channels)
				{
					reinterpret_cast<s16*>(p_buffer[channel])[frames] = static_cast<s16>(*scratch);
				}
				++scratch;
			}
			++frames;
		}
		
		if (result <= 0)
		{
			break;
		}
	}
	
	return frames;
}


snd::size_type StreamMixer::fillBufferInterleaved(snd::size_type  p_frames,
                                                  snd::size_type  p_channels,
                                                  void*           p_buffer,
                                                  snd::size_type* /*p_notifySourceWhenPlaybackReachesThisFrame_OUT*/)
{
	snd::size_type frames = 0;
	while (frames < p_frames)
	{
		// clear xm buffer
		mem::zero32(m_xmBuffer, static_cast<mem::size_type>(m_bufferSize * m_channelCount * sizeof(s32)));
		
		// render xm to buffer
		snd::size_type todo = p_frames - frames;
		snd::size_type result = static_cast<snd::size_type>(mixToBuffer(m_xmBuffer, static_cast<u32>(todo)));
		
		s32* scratch = m_xmBuffer;
		s16* destScratch = reinterpret_cast<s16*>(p_buffer) + (frames * p_channels);
		
		// de-interleave
		for (snd::size_type frame = 0; frame < result; ++frame)
		{
			for (snd::size_type channel = 0; channel < m_channelCount; ++channel)
			{
				// scale down
				*destScratch = static_cast<s16>(*scratch);
				++scratch;
				++destScratch;
			}
			++frames;
		}
		
		if (result <= 0)
		{
			break;
		}
	}
	
	return frames;
}


snd::size_type StreamMixer::getSampleSize() const
{
	return 16;
}


snd::size_type StreamMixer::getBufferSize() const
{
	return m_bufferSize;
}


snd::size_type StreamMixer::getFramerate() const
{
	return m_framerate;
}


snd::size_type StreamMixer::getChannelCount() const
{
	return m_channelCount;
}

// Namespace end
}
}
}
