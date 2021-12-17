#include <cstring>

#include <tt/audio/chibi/OpenALMixer.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace chibi {


OpenALMixer::OpenALMixer(int p_sampleRate, int p_ups, int p_buffers)
:
XMSoftwareMixer(static_cast<u32>(p_sampleRate), 32),
m_buffers(0),
m_source(0),
#ifdef TT_PLATFORM_WIN
m_bufferCount(p_buffers),
#else
m_bufferCount(0),
#endif
m_sampleRate(p_sampleRate),
m_bufferFrames(p_sampleRate / p_ups),
m_bufferSize(0),
m_active(false),
m_pcmBuffer(0),
m_xmBuffer(0),
m_volume(0)
{
	if (p_buffers < 2)
	{
		TT_PANIC("Buffer count should be at least 2.");
		p_buffers = 2;
	}
	
	m_bufferSize = m_bufferFrames * ChannelCount;
	m_pcmBuffer = new s16[m_bufferSize];
	if (m_pcmBuffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_bufferSize * sizeof(s16));
		return;
	}
	
	m_xmBuffer  = new s32[m_bufferSize];
	if (m_xmBuffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_bufferSize * sizeof(s32));
		return;
	}
	
#ifndef TT_PLATFORM_WIN
	if (initializeBuffers(p_buffers) == false)
	{
		return;
	}
#endif
	
	if (initializeSource() == false)
	{
		return;
	}
}


OpenALMixer::~OpenALMixer()
{
	// Stop playing the file
	stop();
	
	clearSource();
	
#ifndef TT_PLATFORM_WIN
	clearBuffers();
#endif
	
	delete[] m_pcmBuffer;
	delete[] m_xmBuffer;
}


void OpenALMixer::stop()
{
	if (playing() == false)
	{
		return;
	}
	
	if (alIsSource(m_source) == AL_FALSE)
	{
		return;
	}
	
	alSourceStop(m_source);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_NAME:
		TT_PANIC("The source is invalid.");
		break;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		break;
		
	default:
		TT_WARN("Unexpected error by alSourceStop (%d)", alGetError());
		break;
	}
	
	empty();
	m_active = false;
	
#ifdef TT_PLATFORM_WIN
	clearBuffers();
#endif
}


void OpenALMixer::setVolume(real p_volume)
{
	if(alIsSource(m_source) == AL_FALSE)
	{
		return;
	}
	
	m_volume = p_volume;
	
	alSourcef(m_source, AL_GAIN, p_volume);
}

void OpenALMixer::play()
{
	if (alIsSource(m_source) == AL_FALSE)
	{
		TT_WARN("Invalid source.");
		return;
	}
	
	// Stop playback if already going on
	stop();
	
#ifdef TT_PLATFORM_WIN
	initializeBuffers(m_bufferCount);
#endif
	
	for (int i = 0; i < m_bufferCount; ++i)
	{
		if (stream(m_buffers[i]) == false)
		{
			TT_WARN("Failed to fill buffers.");
			return;
		}
	}
	
	alSourceQueueBuffers(m_source, m_bufferCount, m_buffers);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_NAME:
		TT_PANIC("The buffer or the source are not valid.");
		break;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context, an attempt was made to add "
		         "a new buffer which is not the same format as the buffers "
		         "already in the queue, or the source already has a static buffer.");
		break;
		
	default:
		TT_WARN("Unexpected error by alSourceQueueBuffers (%d).", alGetError());
		break;
	}
	
	alSourcePlay(m_source);
	m_active = true;
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_NAME:
		TT_PANIC("The source is not valid.");
		break;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		break;
		
	default:
		TT_WARN("Unexpected error by alSourcePlay (%d).", alGetError());
		break;
	}
}


void OpenALMixer::pause()
{
	if (alIsSource(m_source) == AL_FALSE)
	{
		TT_WARN("Invalid source.");
		return;
	}
	
	ALint state = 0;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR: break;
	case AL_INVALID_VALUE:     TT_PANIC("The value pointer given is not valid.");   return;
	case AL_INVALID_ENUM:      TT_PANIC("The specified parameter is not valid.");   return;
	case AL_INVALID_NAME:      TT_PANIC("The specified source name is not valid."); return;
	case AL_INVALID_OPERATION: TT_PANIC("There is no current context.");            return;
	default:                   TT_PANIC("Unexpected error %d.", error);             return;
	}
	
	if (state != AL_PLAYING)
	{
		return;
	}
	
	alSourcePause(m_source);
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR:                                                      return;
	case AL_INVALID_NAME:      TT_PANIC("Specified source is not valid."); return;
	case AL_INVALID_OPERATION: TT_PANIC("There is no current context.");   return;
	default:                   TT_PANIC("Unexpected error %d.", error);    return;
	}
}


void OpenALMixer::resume()
{
	if (alIsSource(m_source) == AL_FALSE)
	{
		TT_WARN("Invalid source.");
		return;
	}
	
	ALint state = 0;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR: break;
	case AL_INVALID_VALUE:     TT_PANIC("The value pointer given is not valid.");   return;
	case AL_INVALID_ENUM:      TT_PANIC("The specified parameter is not valid.");   return;
	case AL_INVALID_NAME:      TT_PANIC("The specified source name is not valid."); return;
	case AL_INVALID_OPERATION: TT_PANIC("There is no current context.");            return;
	default:                   TT_PANIC("Unexpected error %d.", error);             return;
	}
	
	if (state != AL_PAUSED)
	{
		return;
	}
	
	alSourcePlay(m_source);
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR:                                                      return;
	case AL_INVALID_NAME:      TT_PANIC("Specified source is not valid."); return;
	case AL_INVALID_OPERATION: TT_PANIC("There is no current context.");   return;
	default:                   TT_PANIC("Unexpected error %d.", error);    return;
	}
}


bool OpenALMixer::update()
{
	if (alIsSource(m_source) == AL_FALSE)
	{
		return false;
	}
	if (m_active == false)
	{
		return false;
	}
#ifdef TT_PLATFORM_WIN
	if (m_buffers == 0)
	{
		return false;
	}
#endif
	
	ALint processed;
	
	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The specified value pointer is not valid.");
		return false;
		
	case AL_INVALID_ENUM:
		TT_PANIC("The specified parameter is not valid.");
		return false;
		
	case AL_INVALID_NAME:
		TT_PANIC("The specified source is not valid.");
		return false;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alGetSourcei (%d).", alGetError());
		return false;
	}
	
	for (; processed > 0; --processed)
	{
		ALuint buffer;
		
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		switch (alGetError())
		{
		case AL_NO_ERROR:
			break;
			
		case AL_INVALID_VALUE:
			TT_PANIC("Buffer can not be unqueued because it has not been processed yet.");
			return false;
			
		case AL_INVALID_NAME:
			TT_PANIC("The specified source is not valid.");
			return false;
			
		case AL_INVALID_OPERATION:
			TT_PANIC("There is no current context.");
			return false;
			
		default:
			TT_WARN("Unexpected error by alSourceUnqueueBuffers (%d).", alGetError());
			return false;
		}
		
		m_active = stream(buffer);
		
		if (m_active)
		{
			alSourceQueueBuffers(m_source, 1, &buffer);
			switch (alGetError())
			{
			case AL_NO_ERROR:
				break;
				
			case AL_INVALID_VALUE:
				TT_PANIC("Buffer can not be unqueued because it has not been processed yet.");
				return false;
				
			case AL_INVALID_NAME:
				TT_PANIC("The specified source is not valid.");
				return false;
				
			case AL_INVALID_OPERATION:
				TT_PANIC("There is no current context.");
				return false;
				
			default:
				TT_WARN("Unexpected error by alSourceQueueBuffers (%d).", alGetError());
				return false;
			}
		}
		else
		{
			stop();
			return false;
		}
	}
	
	ALint queued;
	alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
			
	case AL_INVALID_VALUE:
		TT_PANIC("The specified value pointer is not valid.");
		return false;
		
	case AL_INVALID_ENUM:
		TT_PANIC("The specified parameter is not valid.");
		return false;
		
	case AL_INVALID_NAME:
		TT_PANIC("The specified source is not valid.");
		return false;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alGetSourcei (%d).", alGetError());
		return false;
	}
	
	if (queued == 0)
	{
		for (int i = 0; i < m_bufferCount; ++i)
		{
			if (stream(m_buffers[i]) == false)
			{
				m_active = false;
				stop();
				return false;
			}
		}
		
		alSourceQueueBuffers(m_source, m_bufferCount, m_buffers);
		switch (alGetError())
		{
		case AL_NO_ERROR:
			break;
			
		case AL_INVALID_NAME:
			TT_PANIC("The buffer or the source are not valid.");
			break;
			
		case AL_INVALID_OPERATION:
			TT_PANIC("There is no current context, an attempt was made to add "
			         "a new buffer which is not the same format as the buffers "
			         "already in the queue, or the source already has a static buffer.");
			break;
			
		default:
			TT_WARN("Unexpected error by alSourceQueueBuffers (%d).", alGetError());
			break;
		}
	}
	
	if (m_active && playing() == false)
	{
		alSourcePlay(m_source);
		switch (alGetError())
		{
		case AL_NO_ERROR:
			break;
			
		case AL_INVALID_NAME:
			TT_PANIC("The source is not valid.");
			break;
			
		case AL_INVALID_OPERATION:
			TT_PANIC("There is no current context.");
			break;
			
		default:
			TT_WARN("Unexpected error by alSourcePlay (%d).", alGetError());
			break;
		}
	}
	
	return m_active;
}


// Private functions

bool OpenALMixer::playing()
{
	ALenum state;
	
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The specified value pointer is not valid.");
		return false;
		
	case AL_INVALID_ENUM:
		TT_PANIC("The specified parameter is not valid.");
		return false;
		
	case AL_INVALID_NAME:
		TT_PANIC("The specified source is not valid.");
		return false;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alGetSourcei (%d).", alGetError());
		return false;
	}
	
	return (state == AL_PLAYING || state == AL_PAUSED);
}


bool OpenALMixer::stream(ALuint p_buffer)
{
	ALsizei size = 0;
	while (size < m_bufferSize)
	{
		// clear xm buffer
		std::memset(m_xmBuffer, 0, m_bufferSize * sizeof(s32));
		
		// render xm to buffer
		u32 todo = static_cast<u32>((m_bufferSize - size) / ChannelCount);
		ALsizei result = static_cast<ALsizei>(mixToBuffer(m_xmBuffer, todo));
		
		// frames to samples
		result *= ChannelCount;
		
		for (int i = 0; i < result; ++i)
		{
			// shift 32 bit samples down to 16 bit
			m_pcmBuffer[size + i] = static_cast<s16>(m_xmBuffer[i]);
		}
		
		if (result > 0)
		{
			size += result;
		}
		else
		{
			break;
		}
	}
	
	if (size == 0)
	{
		// no more data remains
		return false;
	}
	
	// pass data to OpenAL
	alBufferData(p_buffer,
	             AL_FORMAT_STEREO16,
	             m_pcmBuffer,
	             size * sizeof(s16),
	             static_cast<ALsizei>(m_sampleRate));
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_OUT_OF_MEMORY:
		TT_PANIC("There is not enough memory available to create the buffer.");
		return false;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The size parameter is not valid for the specified format, the buffer is in use, "
		         "or the data is a null pointer.");
		return false;
		
	case AL_INVALID_ENUM:
		TT_PANIC("The specified format does not exist.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alBufferData (%d).", alGetError());
		return false;
	}
	
	return true;
}


void OpenALMixer::empty()
{
	int queued;
	
	alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The specified value pointer is not valid.");
		return;
		
	case AL_INVALID_ENUM:
		TT_PANIC("The specified parameter is not valid.");
		return;
		
	case AL_INVALID_NAME:
		TT_PANIC("The specified source is not valid.");
		return;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		return;
		
	default:
		TT_WARN("Unexpected error by alGetSourcei (%d)", alGetError());
		return;
	}
	
	while (queued--)
	{
		ALuint buffer;
		
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		switch (alGetError())
		{
		case AL_NO_ERROR:
			break;
			
		case AL_INVALID_VALUE:
			TT_PANIC("Buffer can not be unqueued because it has not been processed yet.");
			return;
			
		case AL_INVALID_NAME:
			TT_PANIC("The specified source is not valid.");
			return;
			
		case AL_INVALID_OPERATION:
			TT_PANIC("There is no current context.");
			return;
			
		default:
			TT_WARN("Unexpected error by alSourceUnqueueBuffers (%d)", alGetError());
			return;
		}
	}
}


bool OpenALMixer::initializeBuffers(int p_bufferCount)
{
	if (m_buffers != 0)
	{
#ifndef TT_PLATFORM_WIN
		TT_PANIC("Buffers already initialized.");
#endif
		return false;
	}
	
	m_bufferCount = p_bufferCount;
	m_buffers = new ALuint[m_bufferCount];
	if (m_buffers == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_bufferCount * sizeof(ALuint));
		return false;
	}
	
	alGenBuffers(static_cast<ALsizei>(m_bufferCount), m_buffers);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		return true;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The buffer array isn't large enough to hold %d buffers.", m_bufferCount);
		break;
		
	case AL_OUT_OF_MEMORY:
		TT_PANIC("There is not enough memory available to generate %d buffers.", m_bufferCount);
		break;
		
	default:
		TT_WARN("Unexpected error by alGenBuffers (%d)", alGetError());
		break;
	}
	
	tt::code::helpers::safeDeleteArray(m_buffers);
	m_bufferCount = 0;
	return false;
}


bool OpenALMixer::clearBuffers()
{
	if (m_buffers == 0)
	{
		return true;
	}
	
	alDeleteBuffers(static_cast<ALsizei>(m_bufferCount), m_buffers);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("One of the buffers is still in use and cannot be deleted.");
		return false;
		
	case AL_INVALID_NAME:
		TT_PANIC("An invalid buffer has been specified.");
		return false;
		
	case AL_INVALID_VALUE:
		TT_PANIC("The requested number of buffers (%d) cannot be deleted.", m_bufferCount);
		return false;
		
	default:
		TT_WARN("Unexpected error by alDeleteBuffers (%d)", alGetError());
		return false;
	}
	
	tt::code::helpers::safeDeleteArray(m_buffers);
#ifndef TT_PLATFORM_WIN
	m_bufferCount = 0;
#endif
	
	return true;
}


bool OpenALMixer::initializeSource()
{
	if (alIsSource(m_source) == AL_TRUE)
	{
		TT_PANIC("Source already initialized.");
		return false;
	}
	
	alGenSources(1, &m_source);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_OUT_OF_MEMORY:
		TT_PANIC("There is not enough memory to generate the requested source.");
		return false;
		
	case AL_INVALID_VALUE:
		TT_PANIC("There are not enough resources available or the array pointer is invalid.");
		return false;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no context to create sources in.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alGenSources (%d)", alGetError());
		return false;
	}
	
	alSource3f(m_source, AL_POSITION,        0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_VELOCITY,        0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_DIRECTION,       0.0f, 0.0f, 0.0f);
	alSourcef (m_source, AL_ROLLOFF_FACTOR,  0.0f          );
	alSourcei (m_source, AL_SOURCE_RELATIVE, AL_TRUE      );
	
	return true;
}


bool OpenALMixer::clearSource()
{
	if (alIsSource(m_source) == AL_FALSE)
	{
		return true;
	}
	
	alDeleteSources(1, &m_source);
	switch (alGetError())
	{
	case AL_NO_ERROR:
		break;
		
	case AL_INVALID_NAME:
		TT_PANIC("The specified source is not valid or more sources are deleted than exist.");
		return false;
		
	case AL_INVALID_OPERATION:
		TT_PANIC("There is no current context.");
		return false;
		
	default:
		TT_WARN("Unexpected error by alDeleteSources (%d)", alGetError());
		return false;
	}
	
	m_source = 0;
	return true;
}

} // namespace end
}
}
