#include <algorithm>
#include <vector>

#if !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
#include <al/efx-creative.h>
#include <al/EFX-Util.h>
#endif

#include <tt/audio/helpers.h>
#include <tt/audio/xact/RPCCurve.h>
#include <tt/math/math.h>
#include <tt/math/Vector3.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/Buffer.h>
#include <tt/snd/OpenALSoundSystem.h>
#include <tt/snd/snd.h>
#include <tt/snd/Stream.h>
#include <tt/snd/StreamSource.h>
#include <tt/snd/utils.h>
#include <tt/snd/Voice.h>
#include <tt/system/Time.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/thread.h>


namespace tt {
namespace snd {

//--------------------------------------------------------------------------------------------------
// Helper functions


static const char* getALErrorMessage(ALenum p_error)
{
	switch (p_error)
	{
	case AL_NO_ERROR:          return "no error";
	case AL_INVALID_NAME:      return "a bad name (ID) was passed to an OpenAL function";
	case AL_INVALID_ENUM:      return "an invalid enum value was passed to an OpenAL function";
	case AL_INVALID_VALUE:     return "an invalid value was passed to an OpenAL function";
	case AL_INVALID_OPERATION: return "the requested operation is not valid";
	case AL_OUT_OF_MEMORY:     return "the requested operation resulted in OpenAL running out of memory";
	default:                   return "<unknown error>";
	}
}


static const char* getALCErrorMessage(ALCenum p_error)
{
	switch (p_error)
	{
	case ALC_NO_ERROR:        return "None.";
	case ALC_INVALID_DEVICE:  return "Invalid device.";
	case ALC_INVALID_CONTEXT: return "Invalid context.";
	case ALC_INVALID_ENUM:    return "Invalid enum.";
	case ALC_INVALID_VALUE:   return "Invalid value.";
	case ALC_OUT_OF_MEMORY:   return "Out of memory.";
	default:                  return "Unknown error.";
	}
}


#if defined(TT_PLATFORM_OSX_MAC) && !defined(TT_FORCE_OPENAL_SOFT) // this helper is currently only used on Mac OS X: prevent compiler complaints on other platforms
static const ALCchar* alcstr(const char* p_str)
{
	return reinterpret_cast<const ALCchar*>(p_str);
}
#endif


static bool checkALError()
{
	ALenum error = alGetError();

	switch(error)
	{
	case AL_NO_ERROR: return false;
		
	default:
		TT_PANIC("OpenAL Error! (%d): '%s'", error, getALErrorMessage(error));
		return true;
	}
}


static bool isValidSource(ALuint p_source)
{
	return p_source != static_cast<ALuint>(AL_INVALID) &&
	       alIsSource(p_source) != AL_FALSE;
}


static bool playOpenALSource(ALuint p_source)
{
	alSourcePlay(p_source);

	// Can fail if we have exhausted hardware limits
	ALenum error = alGetError();

	switch(error)
	{
	case AL_NO_ERROR         : return true;
	case AL_INVALID_OPERATION: TT_WARN("alSourcePlay failed, probably reached hardware limit."); break;

	default: TT_PANIC("OpenAL Error! (%d): '%s'", error, getALErrorMessage(error)); break;
	}

	return false;
}


static ALint getALSourcei(ALuint p_source, ALenum p_paramName)
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	ALint ret = static_cast<ALint>(AL_INVALID);
	alGetSourcei(p_source, p_paramName, &ret);
	
	if (checkALError())
	{
		return AL_INVALID;
	}
	
	return ret;
}


#ifdef TT_PLATFORM_OSX_IPHONE
// There seems to be a resource leak in the OpenAL implementation used on the iPhone.
// With every allocated source, the allocation process starts to slow down, up to the point
// where it takes nearly a second to allocate a voice.
// To work around this, we can allocate a certain number of voices at startup, and recycle those.
	#define OPENAL_RESOURCE_LEAK
#endif


#if defined(OPENAL_RESOURCE_LEAK)

// Types used to work around openAL resource leak

enum { SourceCount = 64 };


struct OpenALSoundSystem::OpenALSource
{
	ALuint source;
	bool   used;
	
	OpenALSource()
	:
	source(static_cast<ALuint>(AL_INVALID)),
	used(false)
	{
	}
};

#endif //defined(OPENAL_RESOURCE_LEAK)


// Buffer types

struct OpenALSoundSystem::BufferData
{
	ALuint buffer;
	
	inline BufferData()
	:
	buffer(static_cast<ALuint>(AL_INVALID))
	{ }
	
	inline ~BufferData()
	{
		free();
	}
	
	bool allocate()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		free();
		
		if (buffer != static_cast<ALuint>(AL_INVALID) && alIsBuffer(buffer) != AL_FALSE)
		{
			return (checkALError() == false);
		}
		
		alGenBuffers(1, &buffer);
		
		return (checkALError() == false);
	}
	
	bool free()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (buffer == static_cast<ALuint>(AL_INVALID) || alIsBuffer(buffer) == AL_FALSE)
		{
			return (checkALError() == false);
		}
		
		alDeleteBuffers(1, &buffer);

		bool succeeded = (checkALError() == false);
		buffer = static_cast<ALuint>(AL_INVALID);

		return succeeded;
	}
	
	bool setData(const void* p_data,
	             size_type   p_frames,
	             size_type   p_channels,
	             size_type   p_sampleSize,
	             size_type   p_sampleRate,
	             bool        p_ownership)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		// set up an AL format
		ALenum format;
		if (p_channels == 1)
		{
			if (p_sampleSize == 8)
			{
				format = AL_FORMAT_MONO8;
			}
			else
			{
				format = AL_FORMAT_MONO16;
			}
		}
		else
		{
			if (p_sampleSize == 8)
			{
				format = AL_FORMAT_STEREO8;
			}
			else
			{
				format = AL_FORMAT_STEREO16;
			}
		}
		
		alBufferData(buffer,
		             format,
		             p_data,
		             static_cast<ALsizei>(p_frames * p_channels * (p_sampleSize / 8)),
		             static_cast<ALsizei>(p_sampleRate));
		
		if(checkALError()) return false;
		
		if (p_ownership)
		{
			mem::free(const_cast<void*>(p_data));
		}
		return true;
	}
	
	size_type getChannelCount()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALint channels = 0;
		alGetBufferi(buffer, AL_CHANNELS, &channels);

		return checkALError() ? size_type() : channels;
	}
	
	size_type getSampleSize()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALint bits = 0;
		alGetBufferi(buffer, AL_BITS, &bits);
		
		return checkALError() ? size_type() : bits;
	}
	
	size_type getSampleRate()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALint freq = 0;
		alGetBufferi(buffer, AL_FREQUENCY, &freq);

		return checkALError() ? size_type() : freq;
	}
	
	size_type getFrameCount()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		size_type bits     = getSampleSize();
		size_type channels = getChannelCount();
		
		ALint size = 0;
		alGetBufferi(buffer, AL_SIZE, &size);

		if(checkALError()) return size_type();
		
		return size / ((bits / 8) * channels);
	}
	
private:
	// No copying
	BufferData(const BufferData&);
	BufferData& operator=(const BufferData&);
};


// Voice types

struct OpenALSoundSystem::VoiceData
{
	ALuint    source;
	BufferPtr buffer;
	ALfloat   position[3];
	real      panning;
	real      panning360;
	bool      positional;
	
	OpenALSoundSystem* ss;
	
	
	explicit inline VoiceData(OpenALSoundSystem* p_ss)
	:
	source(static_cast<ALuint>(AL_INVALID)),
	buffer(),
	panning(0.0f),
	panning360(0),
	positional(false),
	ss(p_ss)
	{
		position[0] = 0.0f; // X
		position[1] = 0.0f; // Y
		position[2] = 0.0f; // Z
	}
	
	inline ~VoiceData()
	{
		free();
	}
	
	bool allocate()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		free();
		
		if (isValidSource(source))
		{
			return true;
		}
		
		source = ss->allocateSource();
		if (source == static_cast<ALuint>(AL_INVALID))
		{
			return false;
		}

		// Initialize as non-positional
		alSourcei  (source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f (source, AL_POSITION, 0.0f, 0.0f, 0.0f);
		alSource3f (source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

		return (checkALError() == false);
	}
	
	bool free()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isPlaying())
		{
			stop();
		}
		
		if (buffer != 0)
		{
			buffer.reset();
			setBuffer(buffer);
		}
		
		if (isValidSource(source) == false)
		{
			return true;
		}
		
		ss->freeSource(source);
		source = static_cast<ALuint>(AL_INVALID);
		
		return true;
	}
	
	bool isPlaying()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isValidSource(source) == false)
		{
			return false;
		}
		
		ALint state(AL_INVALID);
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		switch (ALenum error = alGetError())
		{
		case AL_NO_ERROR:
			break;
			
		default:
#if defined(TT_BUILD_DEV)
			TT_PANIC("alSourcei (state) failed (%d): '%s'", error, getALErrorMessage(error));
#else
			(void)error;
#endif
			return false;
		}
		return state == AL_PLAYING || state == AL_PAUSED;
	}
	
	bool isPaused()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALint state(AL_INVALID);
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		
		if(checkALError()) return false;

		return state == AL_PAUSED;
	}
	
	bool play(bool p_looping)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (buffer == 0)
		{
			TT_PANIC("Attempt to play voice without buffer.");
			return false;
		}
		
		if (isPlaying())
		{
			TT_WARN("Attempt to play voice that's already playing.");
			return false;
		}
		
		alSourcei(source, AL_LOOPING, static_cast<ALint>(p_looping ? AL_TRUE : AL_FALSE));

		if(checkALError()) return false;
		
		return playOpenALSource(source);
	}
	
	bool stop()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isPlaying() == false)
		{
			return true;
		}
		
		alSourceStop(source);

		return (checkALError() == false);
	}
	
	bool pause()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isPlaying() == false)
		{
			TT_WARN("Attempt to pause a voice that's not playing.");
			return false;
		}
		
		if (isPaused())
		{
			return true;
		}
		alSourcePause(source);
		return (checkALError() == false);
	}
	
	bool resume()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isPlaying() == false)
		{
			TT_WARN("Attempt to resume a voice that's not playing.");
			return false;
		}
		
		if (isPaused() == false)
		{
			return true;
		}
		
		return playOpenALSource(source);
	}
	
	bool setBuffer(const BufferPtr& p_buffer)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isPlaying())
		{
			TT_WARN("Stop voice before assigning a new buffer.");
			return false;
		}
		
		if (isValidSource(source) == false)
		{
			TT_WARN("Can't set buffer on invalid source");
			return false;
		}
		
		if (p_buffer == 0)
		{
			alSourcei(source, AL_BUFFER, AL_NONE);
		}
		else
		{
			BufferData* data = reinterpret_cast<BufferData*>(p_buffer->getData());
			if (data == 0)
			{
				TT_PANIC("Buffer has not been initialized.");
				return false;
			}
			TT_ASSERT(data->buffer != 0);
			alSourcei(source, AL_BUFFER, data->buffer);
		}
		if(checkALError()) return false;

		buffer = p_buffer;
		return true;
	}
	
	real getPlaybackRatio()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALfloat pitch(0.0f);
		alGetSourcef(source, AL_PITCH, &pitch);

		return checkALError() ? 0.0f : static_cast<real>(pitch);
	}
	
	bool setPlaybackRatio(real p_ratio)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		alSourcef(source, AL_PITCH, static_cast<ALfloat>(p_ratio));

		return (checkALError() == false);
	}
	
	real getVolumeRatio()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		ALfloat gain(0.0f);
		alGetSourcef(source, AL_GAIN, &gain);

		return checkALError() ? 0.0f : static_cast<real>(gain);
	}

	
	bool setVolumeRatio(real p_ratio)
	{
		static const real maxVolumeRatio = audio::helpers::dBToRatio(6.0f);

		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (p_ratio < 0.0f)
		{
			TT_WARN("Clamping volume (%f).", p_ratio);
			p_ratio = 0.0f;
		}
		else if (p_ratio > maxVolumeRatio)
		{
			TT_WARN("Clamping volume (%f).", p_ratio);
			p_ratio = maxVolumeRatio;
		}
		
		if (source == static_cast<ALuint>(AL_INVALID))
		{
			// Invalid source, ignore.
			return true;
		}
		
		alSourcef(source, AL_GAIN, static_cast<ALfloat>(p_ratio));

		return (checkALError() == false);
	}
	
	inline real getPanning() const
	{
		return panning;
	}
	
	bool setPanning(real p_panning)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		position[0] = static_cast<ALfloat>(p_panning);
		position[2] = 0.0f;
		panning     = p_panning;
		// approximation, but it does the trick
		if (p_panning >= 0.0f)
		{
			panning360 = 90.0f * p_panning;
		}
		else
		{
			panning360 = 360.0f - (90.0f * p_panning);
		}
		alSourcefv(source, AL_POSITION, position);

		return (checkALError() == false);
	}
	
	inline real get360Panning() const
	{
		return panning360;
	}
	
	bool set360Panning(real p_panning)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		panning360 = static_cast<real>(static_cast<int>(p_panning) % 360);
		real angle = math::degToRad(panning360);
		
		position[0] = static_cast<ALfloat>(math::sin(angle));
		position[2] = static_cast<ALfloat>(math::cos(angle));
		
		// approximation, but it does the trick
		panning     = static_cast<real>(position[0]);
		
		alSourcefv(source, AL_POSITION, position);

		return (checkALError() == false);
	}


	bool makePositional()
	{
		alSourcei (source, AL_SOURCE_RELATIVE, AL_FALSE);

		// Positional audio config
		alSourcef(source, AL_REFERENCE_DISTANCE, 10.0f);  // inner radius
		alSourcef(source, AL_MAX_DISTANCE,       25.0f);  // outer radius
		alSourcef(source, AL_ROLLOFF_FACTOR,     1.0f);

		positional = true;

		return (checkALError() == false);
	}


	bool setPosition(const math::Vector3& p_position)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (positional == false)
		{
			if (makePositional() == false)
			{
				return false;
			}
		}
		
		position[0] = static_cast<ALfloat>(p_position.x);
		position[1] = static_cast<ALfloat>(p_position.y);
		position[2] = static_cast<ALfloat>(p_position.z);
		
		// approximation, but it does the trick
		panning     = static_cast<real>(position[0]);
		
		// Set emitter position
		alSourcefv(source, AL_POSITION, position);
		
		return (checkALError() == false);
	}


	bool setRadius(real p_inner, real p_outer)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (positional == false)
		{
			if (makePositional() == false)
			{
				return false;
			}
		}
		
		alSourcef(source, AL_REFERENCE_DISTANCE, p_inner);
		alSourcef(source, AL_MAX_DISTANCE,       p_outer);
		
		return (checkALError() == false);
	}


	void setDistanceProperties(real p_reference, real p_max, real p_rolloff)
	{
		alSourcef(source, AL_REFERENCE_DISTANCE, p_reference);
		alSourcef(source, AL_MAX_DISTANCE, p_max);
		alSourcef(source, AL_ROLLOFF_FACTOR, p_rolloff);
	}
	
private:
	// No copying
	VoiceData(const VoiceData&);
	VoiceData& operator=(const VoiceData&);
};


// Stream types

enum { BufferCount = 2 };

struct OpenALSoundSystem::StreamData
{
public:
	inline explicit StreamData(StreamSource* p_streamSource)
	:
	m_buffer(0),
	m_buffers(),
	m_source(static_cast<ALuint>(AL_INVALID)),
	m_format(0),
	m_frames(p_streamSource->getBufferSize() / BufferCount),
	m_channels(p_streamSource->getChannelCount()),
	m_bufferSize((p_streamSource->getBufferSize() / BufferCount) * p_streamSource->getChannelCount() * p_streamSource->getSampleSize() / 8),
	m_volumeRatio(1.0f),
	m_volumedB(0.0f),
	m_playing(false),
	m_paused(false),
	m_playbackDone(false),
	m_bufferLengthInMs(static_cast<u32>((p_streamSource->getBufferSize() * 1000) / p_streamSource->getFramerate()))
	{
		TT_ASSERT(m_buffer == 0);
		m_buffer = ::operator new(m_bufferSize);
		m_buffers.resize(BufferCount, 0);
		
		switch (size_type channelCount = p_streamSource->getChannelCount())
		{
		case 1:
			switch (size_type sampleSize = p_streamSource->getSampleSize())
			{
			case 8: m_format = AL_FORMAT_MONO8; break;
			default:
				TT_PANIC("Unsupported sample size %d, scaling to 16.", sampleSize);
			case 16:
				m_format = AL_FORMAT_MONO16;
				break;
			}
			break;
			
		default:
			TT_PANIC("Unsupported channel count %d, scaling to 2.", channelCount);
		case 2:
			switch (size_type sampleSize = p_streamSource->getSampleSize())
			{
			case 8: m_format = AL_FORMAT_STEREO8; break;
			default:
				TT_PANIC("Unsupported sample size %d, scaling to 16.", sampleSize);
			case 16:
				m_format = AL_FORMAT_STEREO16;
				break;
			}
			break;
		}
	}
	
	inline ~StreamData()
	{
		::operator delete(m_buffer);
	}
	
	bool allocate(ALuint p_alSource, StreamSource* p_streamSource)
	{
		TT_NULL_ASSERT(p_streamSource);
		TT_ASSERT(m_source == static_cast<ALuint>(AL_INVALID));
		
		if (isValidSource(p_alSource) == false)
		{
			TT_PANIC("Invalid OpenAL sound source passed for stream.");
			return false;
		}
		
		m_source = p_alSource;
		
		alSource3f(m_source, AL_POSITION,        0.0f, 0.0f, 0.0f);
		alSource3f(m_source, AL_VELOCITY,        0.0f, 0.0f, 0.0f);
		alSource3f(m_source, AL_DIRECTION,       0.0f, 0.0f, 0.0f);
		alSourcef (m_source, AL_ROLLOFF_FACTOR,  1.0f            );
		alSourcei (m_source, AL_SOURCE_RELATIVE, AL_TRUE         );
		
		alGenBuffers(static_cast<ALsizei>(m_buffers.size()), &m_buffers[0]);
		if (checkALError()) return false;
		
		// Fill the buffer
		for (Buffers::size_type i = 0; i < m_buffers.size(); ++i)
		{
			if (fillBuffer(p_streamSource, m_buffers[i]) == FillResult_OpenALError)
			{
				TT_PANIC("The initial fillBuffer failed.");
				return false;
			}
		}
		
		alSourceQueueBuffers(m_source, static_cast<ALsizei>(m_buffers.size()), &m_buffers[0]);
		
		if (checkALError())
		{
			return false;
		}
		
		return true;
	}
	
	bool free()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		// Make sure source is stopped.
		ALint state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
		checkALError();
		if (state != AL_STOPPED)
		{
			alSourceStop(m_source);
			checkALError();
		}
		
		// Detach all buffers (OpenAL_Programmers_Guide - page 14 - Queuing Buffers on a Source:
		//                     "Any source can have all buffers detached from it using alSourcei(..., AL_BUFFER, 0), [...]". )
		alSourcei(m_source, AL_BUFFER, AL_NONE);
		checkALError();
		
		alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), &m_buffers[0]);
		checkALError();
		
		m_buffers.clear();
		
		return true;
	}
	
	bool play()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (m_playing)
		{
			TT_PANIC("Stream already playing.");
			return false;
		}
		
		if (m_source == static_cast<ALuint>(AL_INVALID)) // FIXME: Should I add this part?: || alIsSource(m_source) == AL_FALSE)
		{
			return false;
		}
		
		bool result = playOpenALSource(m_source);
		TT_ASSERTMSG(result, "Failed to play openAL source.");
		
		m_playing = result;
		m_paused  = (m_playing == false);
		return result;
	}
	
	bool stop()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (m_playing == false)
		{
			TT_WARN("Stream not playing.");
			return true;
		}
		
		TT_ASSERT(isValidSource(m_source));
		
		m_playing = false;
		m_paused  = false;
		
		alSourceStop(m_source);
		if(checkALError()) return false;
		
		
		ALint state = getALSourcei(m_source, AL_SOURCE_STATE);
		TT_ASSERT(state == AL_STOPPED || state == AL_INITIAL);
		checkALError();
		
		return true;
	}
	
	bool pause()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (m_playing == false)
		{
			TT_PANIC("Cannot pause non playing stream.");
			return false;
		}
		
		if (m_paused)
		{
			TT_WARN("Stream already paused.");
			return true;
		}
		TT_ASSERT(isValidSource(m_source));
		
		// Pause the buffer
		alSourcePause(m_source);
		if(checkALError()) return false;
		
		m_paused = true;
		
		return true;
	}
	
	bool resume()
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (m_playing == false)
		{
			TT_PANIC("Cannot resume non playing stream.");
			return false;
		}
		
		if (m_paused == false)
		{
			TT_WARN("Stream already playing.");
			return true;
		}
		TT_ASSERT(isValidSource(m_source));
		
		// Resume playing the buffer
		bool result = playOpenALSource(m_source);
		
		m_paused = (result == false);
		
		return result;
	}
	
	bool setVolumeRatio(real p_volumeRatio)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isValidSource(m_source) == false)
		{
			TT_PANIC("Steam data source not initialized.");
			return false;
		}
		
		const real dB = audio::helpers::ratioTodB(p_volumeRatio);
		
		m_volumeRatio = p_volumeRatio;
		m_volumedB    = dB;
		
		alSourcef(m_source, AL_GAIN, m_volumeRatio);
		
		return (checkALError() == false);
	}
	
	bool setVolumeDB(real p_volumeInDB)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (isValidSource(m_source) == false)
		{
			TT_PANIC("Steam data source not initialized.");
			return false;
		}
		
		const real ratio = audio::helpers::dBToRatio(p_volumeInDB);
		
		m_volumeRatio = ratio;
		m_volumedB    = p_volumeInDB;
		
		alSourcef(m_source, AL_GAIN, m_volumeRatio);
		
		return (checkALError() == false);
	}
	
	bool update(StreamSource* p_source)
	{
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		if (m_playing == false)
		{
			TT_WARN("Updating non playing stream.");
			return true;
		}
		
		const bool streamHasValidSourceVoice = isValidSource(m_source);
		TT_ASSERT(streamHasValidSourceVoice);
		
		if (streamHasValidSourceVoice == false)
		{
			TT_PANIC("Audio stream does not have a valid OpenAL voice: cannot update stream.");
			return false;
		}
		
		// Check play position
		/*
		if (haveNotificationFrame)
		{
			playPosition = getPlayPosition();
			if (playPosition >= notificationFrame)
			{
				p_source->onStreamReachedNotificationFrame();
			}
		}
		*/
		
		ALint processed;
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
		
		if(checkALError()) return false;
		
		for (; processed > 0; --processed)
		{
			ALuint alBuffer;
			
			alSourceUnqueueBuffers(m_source, 1, &alBuffer);
			
			if(checkALError()) return false;
			
			const FillResult result = fillBuffer(p_source, alBuffer);
			if (result == FillResult_OpenALError)
			{
				// FIXME: Should be pause stream, but that's not working.
				// (False from fill buffer is an error, or stop because of non-looping stream.)
				//pauseStream(p_stream);
				return false;
			}
			else if (result == FillResult_Filled)
			{
				alSourceQueueBuffers(m_source, 1, &alBuffer);
				
				if(checkALError()) return false;
			}
		}
		
		if (m_playbackDone)
		{
			// Stream reached end of its buffer: check if playback has also completed
			ALenum state;
			alGetSourcei(m_source, AL_SOURCE_STATE, &state);
			if (checkALError() == false &&
			    state == AL_STOPPED)
			{
				m_playing = false;
				m_paused  = false;
			}
		}
		
#if !defined(TT_BUILD_FINAL)
		if (m_playbackDone == false)
		{
			// Check that we still have buffers queued.
			ALint queued;
			alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
			checkALError();
			TT_ASSERT(queued > 0);
		}
		
		// Check that we still have the correct state.
		ALenum state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
		checkALError();
		
		// Check if state is correct.
		// With threading it can be that the al state and data bools are out of sync for a frame.
		//switch (state)
		//{
		//case AL_INITIAL: TT_ASSERT(paused == false); TT_ASSERT(playing == false); break;
		//case AL_PLAYING: TT_ASSERT(paused == false); TT_ASSERT(playing == true ); break;
		//case AL_PAUSED:  TT_ASSERT(paused == true ); TT_ASSERT(playing == true ); break;
		//case AL_STOPPED: TT_ASSERT(paused == false); TT_ASSERT(playing == false); break;
		//default: TT_PANIC("Unknown openal source state: 0x%X", state);            break;
		//}
#endif
		
		return true;
	}
	
	
	inline bool   isPlaying()           const { return m_playing;          }
	inline bool   isPaused()            const { return m_paused;           }
	inline real   getVolumeRatio()      const { return m_volumeRatio;      }
	inline real   getVolumeDB()         const { return m_volumedB;         }
	inline ALuint getALSource()         const { return m_source;           }
	inline u32    getBufferLengthInMs() const { return m_bufferLengthInMs; }
	
private:
	typedef std::vector<ALuint> Buffers;
	
	enum FillResult
	{
		FillResult_OpenALError,
		FillResult_NoDataToPlay,
		FillResult_Filled
	};
	
	
	FillResult fillBuffer(StreamSource* p_source, ALuint p_buffer)
	{
		if (m_playbackDone)
		{
			return FillResult_NoDataToPlay;
		}
		
		TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
		
		mem::zero8(m_buffer, static_cast<mem::size_type>(m_bufferSize));
		
		size_type notificationFrame = -1; // FIXME: Stream notifications not supported with this sound system yet
		const size_type framesFilled = p_source->fillBufferInterleaved(m_frames, m_channels, m_buffer, &notificationFrame);
		if (framesFilled <= 0)
		{
			return FillResult_NoDataToPlay;
		}
		
		// HACK: Crude stream playback notification. This should actually be triggered if the
		// stream playback position reaches this frame, not when this frame is buffered
		if (notificationFrame >= 0)
		{
			p_source->onStreamReachedNotificationFrame();
		}
		
		// pass data to OpenAL
		alBufferData(p_buffer,
		             m_format,
		             m_buffer,
		             m_bufferSize,
		             static_cast<ALsizei>(p_source->getFramerate()));
		
		if (checkALError())
		{
			return FillResult_OpenALError;
		}
		
		// Source could not completely fill the buffer: assume this to mean the source is empty (so playback should finish)
		if (framesFilled < m_frames)
		{
			m_playbackDone = true;
		}
		
		return FillResult_Filled;
	}
	
	// No copying
	StreamData(const StreamData&);
	StreamData& operator=(const StreamData&);
	
	
	void*   m_buffer;  //!< Buffer for holding streamed data.
	Buffers m_buffers; //!< Buffers to hold data for playback.
	ALuint  m_source;  //!< Source for playback.
	ALenum  m_format;  //!< Sample type.
	
	size_type m_frames;     //!< Number of frames per buffer.
	size_type m_channels;   //!< Number of channels.
	size_type m_bufferSize; //!< Number of bytes per buffer.
	
	real m_volumeRatio; //!< Normalized volume
	real m_volumedB;    //!< Volume in deciBells
	
	bool m_playing;
	bool m_paused;
	bool m_playbackDone;
	
	u32 m_bufferLengthInMs;  // used by the stream decoding thread to estimate whether it can idle
};


SoundSystemPtr OpenALSoundSystem::instantiate(identifier p_identifier, bool p_updateStreamsInThread)
{
	SoundSystemPtr sys(new OpenALSoundSystem(p_identifier, p_updateStreamsInThread));
	if (sys == 0)
	{
		TT_PANIC("Failed to instantiate soundsystem.");
		return SoundSystemPtr();
	}
	
	if (snd::registerSoundSystem(sys.get(), p_identifier) == false)
	{
		TT_PANIC("Failed to register soundsytem.");
		return SoundSystemPtr();
	}
	
	return sys;
}


OpenALSoundSystem::~OpenALSoundSystem()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	if (m_updateStreamsInThread)
	{
		if (m_decodeThread != 0)
		{
			// Tell decode thread to exit and wait for the thread to die
			{
				thread::CriticalSection criticalSection(&m_decodeMutex);
				m_decodeThreadShouldExit = true;
			}
			thread::wait(m_decodeThread); // join
			m_decodeThread.reset();
		}
		
		m_decodeSemaphore.destroy();
	}
	
	freeSources();
	
	// clean up
	alcMakeContextCurrent(NULL);
	
	if (m_fx.supported)
	{
#if defined(TT_FORCE_OPENAL_SOFT)	
#elif defined(TT_PLATFORM_OSX_MAC)
		// No cleanup needed for Mac OS X
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
		if (m_effectSlot != AL_EFFECTSLOT_NULL)
		{
			// Remove effect from slot
			m_fx.alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		}
		
		// Delete Effect
		if (m_effect != AL_EFFECT_NULL)
		{
			m_fx.alDeleteEffects(1, &m_effect);
			m_effect = AL_EFFECT_NULL;
		}
		
		// Delete Auxiliary Effect Slot
		if (m_effectSlot != AL_EFFECTSLOT_NULL)
		{
			m_fx.alDeleteAuxiliaryEffectSlots(1, &m_effectSlot);
			m_effectSlot = AL_EFFECTSLOT_NULL;
		}
#endif
	}
	
	alcDestroyContext(m_context);
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcDestroyContext: %s", getALCErrorMessage(error));
		break;
	}
	
	ALCboolean success = alcCloseDevice(m_device);
	switch (ALCenum error = alcGetError(NULL))
	{
	case ALC_NO_ERROR:
		break;
		
	case ALC_INVALID_DEVICE:
		// Device pointer that was passed is invalid (which makes sense, since it's null): ignore
		break;
		
	default:
		TT_PANIC("alcCloseDevice: %s", getALCErrorMessage(error));
		break;
	}
	
	if (success != ALC_TRUE)
	{
		TT_PANIC("Device contains contexts or buffers.");
	}
}


// Master volume functions

real OpenALSoundSystem::getMasterVolume()
{
	ALfloat volume = 0.0f;
	alGetListenerf(AL_GAIN, &volume);
	return static_cast<real>(volume);
}


void OpenALSoundSystem::setMasterVolume(real p_volume)
{
	real clampedVolume = p_volume;
	if (math::clamp(clampedVolume, 0.0f, 1.0f))
	{
		TT_WARN("Invalid master volume: %f . Must be in range 0.0 - 1.0", p_volume);
	}
	
	alListenerf(AL_GAIN, clampedVolume);
}


// Master suspend functions

bool OpenALSoundSystem::suspend()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	ALCboolean result = alcMakeContextCurrent(0);
	if (result != ALC_TRUE)
	{
		TT_PANIC("Failed to reset OpenAL context.");
		//return false;
	}
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcMakeContextCurrent: %s", getALCErrorMessage(error));
		return false;
	}
	
	alcSuspendContext(m_context);
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcSuspendContext: %s", getALCErrorMessage(error));
		return false;
	}
	
	return true;
}


bool OpenALSoundSystem::resume()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	ALCboolean result = alcMakeContextCurrent(m_context);
	if (result != ALC_TRUE)
	{
		TT_PANIC("Failed to set OpenAL context.");
		//return false;
	}
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcMakeContextCurrent: %s", getALCErrorMessage(error));
		return false;
	}
	
	alcProcessContext(m_context);
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcProcessContext: %s", getALCErrorMessage(error));
		return false;
	}
	
	return true;
}


// Voice creation functions

bool OpenALSoundSystem::openVoice(const VoicePtr& p_voice, size_type p_priority)
{
	(void)p_priority;
	TT_NULL_ASSERT(p_voice);
	if (p_voice->getData() != 0)
	{
		TT_PANIC("Voice already open.");
		return false;
	}
	
	VoiceData* data = new VoiceData(this);
	if (data->allocate() == false)
	{
		delete data;
		return false;
	}
	
	/* DEBUG: Set reverb effect slot for all voices
	if (m_effectSlot != AL_EFFECTSLOT_NULL)
	{
		alSource3i(data->source, AL_AUXILIARY_SEND_FILTER, m_effectSlot, 0, AL_FILTER_NULL);
	}
	// END DEBUG */
	
	p_voice->setData(data);
	
	return true;
}


bool OpenALSoundSystem::closeVoice(Voice* p_voice)
{
	if (p_voice == 0 || p_voice->getData() == 0)
	{
		// Voice data already cleaned up, most likely openVoice() failed
		// This is a legal situation
		return true;
	}
	
	VoiceData* data = reinterpret_cast<VoiceData*>(p_voice->getData());
	
	bool success = true;
	if (data->free() == false)
	{
		TT_PANIC("Failed to close voice.");
		success = false;
		// we'll need to clean up the internal data to prevent more leaks.
	}
	
	delete data;
	p_voice->setData(0);
	
	return success;
}


// Voice Playback functions

bool OpenALSoundSystem::playVoice(const VoicePtr& p_voice, bool p_loop)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->play(p_loop) : false;
}


bool OpenALSoundSystem::stopVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->stop() : false;
}


bool OpenALSoundSystem::pauseVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->pause() : false;
}


bool OpenALSoundSystem::resumeVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->resume() : false;
}


// Voice Status functions

bool OpenALSoundSystem::isVoicePlaying(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->isPlaying() : false;
}


bool OpenALSoundSystem::isVoicePaused(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->isPaused() : false;
}


// Voice Parameter functions

BufferPtr OpenALSoundSystem::getVoiceBuffer(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->buffer : BufferPtr();
}


bool OpenALSoundSystem::setVoiceBuffer(const VoicePtr& p_voice, const BufferPtr& p_buffer)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setBuffer(p_buffer) : false;
}


real OpenALSoundSystem::getVoicePlaybackRatio(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->getPlaybackRatio() : 0.0f;
}


bool OpenALSoundSystem::setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setPlaybackRatio(p_playbackRatio) : false;
}


real OpenALSoundSystem::getVoiceVolumeRatio(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->getVolumeRatio() : 0.0f;
}


bool OpenALSoundSystem::setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setVolumeRatio(p_volumeRatio) : false;
}


real OpenALSoundSystem::getVoicePanning(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->getPanning() : 0.0f;
}


bool OpenALSoundSystem::setVoicePanning(const VoicePtr& p_voice, real p_panning)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setPanning(p_panning) : false;
}


real OpenALSoundSystem::getVoice360Panning(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->get360Panning() : 0;
}


bool OpenALSoundSystem::setVoice360Panning(const VoicePtr& p_voice, real p_panning)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->set360Panning(p_panning) : false;
}


bool OpenALSoundSystem::setVoicePosition(const VoicePtr& p_voice, const math::Vector3& p_position)
{
	if (is3DAudioEnabled() == false)
	{
		TT_PANIC("3D Audio is not enabled, call set3DAudioEnabled() first!");
		return false;
	}
	
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setPosition(p_position) : false;
}


bool OpenALSoundSystem::setVoiceRadius(const VoicePtr& p_voice, real p_inner, real p_outer)
{
	if (is3DAudioEnabled() == false)
	{
		TT_PANIC("3D Audio is not enabled, call set3DAudioEnabled() first!");
		return false;
	}
	
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setRadius(p_inner, p_outer) : false;
}


bool OpenALSoundSystem::setVoiceReverbVolume(const VoicePtr& p_voice, real p_volumeInDB)
{
	VoiceData* data = getVoiceData(p_voice);
	if (data == 0)
	{
		return false;
	}
	
	if (m_fx.supported == false)
	{
		TT_WARN("Cannot set reverb volume: OpenAL audio effects extension not supported.");
		return false;
	}
	
	if (isValidSource(data->source) == false)
	{
		TT_PANIC("Cannot set voice reverb volume: voice does not have a valid OpenAL source.");
		return false;
	}
	
	const real ratio = audio::helpers::dBToRatio(p_volumeInDB);
	
#if defined(TT_FORCE_OPENAL_SOFT)	
	// TODO: Implement this...
	(void)ratio;
	return false;
#elif defined(TT_PLATFORM_OSX_MAC)
	
	ALfloat reverbSendLevel = static_cast<ALfloat>(ratio);
	m_fx.alcASASetSource(m_fx.enumALC_ASA_REVERB_SEND_LEVEL, data->source, &reverbSendLevel, sizeof(reverbSendLevel));
	const ALCenum errorCode = alcGetError(m_device);
	if (errorCode != ALC_NO_ERROR)
	{
		TT_PANIC("Could not set ASA reverb send level.\n"
		         "OpenAL error %d: '%s'", errorCode, getALCErrorMessage(errorCode));
		return false;
	}
	
	return true;
	
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
	
	// TODO: Implement this...
	(void)ratio;
	
	return true;
	
#else
	// Unsupported platform
	TT_WARN("Reverb not supported on this platform.");
	(void)ratio;
	return false;
#endif
}


bool OpenALSoundSystem::setReverbEffect(ReverbPreset p_preset)
{
	if (m_fx.supported == false)
	{
		TT_WARN("Cannot set reverb effect: OpenAL audio effects extension not supported.");
		return false;
	}
	
	if (p_preset == m_activeReverbPreset)
	{
		// Effect did not change: prevent unnecessary work
		return true;
	}
#if defined(TT_FORCE_OPENAL_SOFT)
#elif defined(TT_PLATFORM_OSX_MAC)
	
	ALCenum errorCode = ALC_NO_ERROR;
	
	if (p_preset == ReverbPreset_None)
	{
		// Simply turn reverb off
		u32 setting = 0;
		m_fx.alcASASetListener(m_fx.enumALC_ASA_REVERB_ON, &setting, sizeof(setting));
		errorCode = alcGetError(m_device);
		if (errorCode != ALC_NO_ERROR)
		{
			TT_PANIC("Could not disable ASA reverb.\n"
			         "OpenAL error %d: '%s'", errorCode, getALCErrorMessage(errorCode));
			return false;
		}
		m_activeReverbPreset = p_preset;
		return true;
	}
	
	u32 roomType = 0;
	switch (p_preset)
	{
	case ReverbPreset_SmallRoom:     roomType = ALC_ASA_REVERB_ROOM_TYPE_SmallRoom;  break;
	case ReverbPreset_LargeRoom:     roomType = ALC_ASA_REVERB_ROOM_TYPE_LargeRoom;  break;
	case ReverbPreset_Hall:          roomType = ALC_ASA_REVERB_ROOM_TYPE_MediumHall; break;
	case ReverbPreset_Cathedral:     roomType = ALC_ASA_REVERB_ROOM_TYPE_Cathedral;  break;
	case ReverbPreset_MetalCorridor: roomType = ALC_ASA_REVERB_ROOM_TYPE_Plate;      break;
		
		// Custom presets
	case ReverbPreset_SmallCave:
		// NOTE: Cannot set custom reverb values, so select a preset that matches best...
		roomType = ALC_ASA_REVERB_ROOM_TYPE_SmallRoom;
		break;
		
	case ReverbPreset_LargeCave:
		// NOTE: Cannot set custom reverb values, so select a preset that matches best...
		roomType = ALC_ASA_REVERB_ROOM_TYPE_LargeHall;
		break;
		
	default:
		TT_PANIC("Unsupported reverb preset: %d", p_preset);
		return false;
	}
	
	// Enable reverb
	u32 setting = 1;
	m_fx.alcASASetListener(m_fx.enumALC_ASA_REVERB_ON, &setting, sizeof(setting));
	errorCode = alcGetError(m_device);
	if (errorCode != ALC_NO_ERROR)
	{
		TT_PANIC("Could not enable ASA reverb.\n"
		         "OpenAL error %d: '%s'", errorCode, getALCErrorMessage(errorCode));
		return false;
	}
	
	// Set the reverb type
	m_fx.alcASASetListener(m_fx.enumALC_ASA_REVERB_ROOM_TYPE, &roomType, sizeof(roomType));
	errorCode = alcGetError(m_device);
	if (errorCode != ALC_NO_ERROR)
	{
		TT_PANIC("Could not set ASA room type to %u\n"
		         "OpenAL error %d: '%s'", roomType, errorCode, getALCErrorMessage(errorCode));
		return false;
	}
	
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
	
	// Clear AL error state
	alGetError();
	
	ALenum errorCode = AL_NO_ERROR;
	
	if (p_preset == ReverbPreset_None)
	{
		// Disable the reverb effect
		if (m_effectSlot != AL_EFFECTSLOT_NULL)
		{
			m_fx.alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			errorCode = alGetError();
			if (errorCode != AL_NO_ERROR)
			{
				TT_PANIC("Could not remove reverb effect from auxiliary effect slot.\n"
				         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
				return false;
			}
		}
		
		m_activeReverbPreset = p_preset;
		return true;
	}
	
	// Determine the reverb settings to use, before creating any resources
	// (so that no unnecessary work was performed in case of unsupported preset value)
	EFXEAXREVERBPROPERTIES reverbParams;
	switch (p_preset)
	{
	case ReverbPreset_SmallRoom:
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_WOODEN_SMALLROOM; 
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	case ReverbPreset_LargeRoom:
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_WOODEN_LARGEROOM;
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	case ReverbPreset_Hall:
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_HALLWAY;
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	case ReverbPreset_Cathedral:
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_CHAPEL;
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	case ReverbPreset_MetalCorridor:
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_PIPE_LONGTHIN;
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
		// Custom presets
	case ReverbPreset_SmallCave:
		// FIXME: Need tweaked custom settings
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_CAVE; //REVERB_PRESET_PSYCHOTIC
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	case ReverbPreset_LargeCave:
		// FIXME: Need tweaked custom settings
		{
			EAXREVERBPROPERTIES eaxProps = REVERB_PRESET_CAVE; //REVERB_PRESET_BATHROOM
			ConvertReverbParameters(&eaxProps, &reverbParams);
		}
		break;
		
	default:
		TT_PANIC("Unsupported reverb preset: %d", p_preset);
		return false;
	}
	
	// Generate an Auxiliary Effect Slot (if we didn't have one yet)
	if (m_effectSlot == AL_EFFECTSLOT_NULL)
	{
		m_fx.alGenAuxiliaryEffectSlots(1, &m_effectSlot);
		errorCode = alGetError();
		if (errorCode != AL_NO_ERROR)
		{
			TT_PANIC("Could not create auxiliary effect slot for reverb effect.\n"
			         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
			m_effectSlot = AL_EFFECTSLOT_NULL;
			return false;
		}
	}
	
	// Generate an effect object (if we didn't have one yet)
	if (m_effect == AL_EFFECT_NULL)
	{
		m_fx.alGenEffects(1, &m_effect);
		errorCode = alGetError();
		if (errorCode != AL_NO_ERROR)
		{
			TT_PANIC("Could not create an effect object for reverb effect.\n"
			         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
			m_effect = AL_EFFECT_NULL;
			return false;
		}
		
		// Set the effect type to reverb (perhaps use AL_EFFECT_REVERB? what's the difference?)
		m_fx.alEffecti(m_effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
		errorCode = alGetError();
		if (errorCode != AL_NO_ERROR)
		{
			TT_PANIC("Could not set effect object type for reverb effect.\n"
			         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
			m_fx.alDeleteEffects(1, &m_effect);
			m_effect = AL_EFFECT_NULL;
			return false;
		}
	}
	
	// Effect slot and effect object must have been created at this point
	TT_ASSERT(m_effectSlot != AL_EFFECTSLOT_NULL);
	TT_ASSERT(m_effect != AL_EFFECT_NULL);
	
	// Set the effect parameters
	m_fx.alEffectf (m_effect, AL_EAXREVERB_DENSITY,               reverbParams.flDensity);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_DIFFUSION,             reverbParams.flDiffusion);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_GAIN,                  reverbParams.flGain);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_GAINHF,                reverbParams.flGainHF);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_GAINLF,                reverbParams.flGainLF);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_DECAY_TIME,            reverbParams.flDecayTime);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_DECAY_HFRATIO,         reverbParams.flDecayHFRatio);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_DECAY_LFRATIO,         reverbParams.flDecayLFRatio);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_REFLECTIONS_GAIN,      reverbParams.flReflectionsGain);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_REFLECTIONS_DELAY,     reverbParams.flReflectionsDelay);
	m_fx.alEffectfv(m_effect, AL_EAXREVERB_REFLECTIONS_PAN,       reverbParams.flReflectionsPan);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_LATE_REVERB_GAIN,      reverbParams.flLateReverbGain);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_LATE_REVERB_DELAY,     reverbParams.flLateReverbDelay);
	m_fx.alEffectfv(m_effect, AL_EAXREVERB_LATE_REVERB_PAN,       reverbParams.flLateReverbPan);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_ECHO_TIME,             reverbParams.flEchoTime);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_ECHO_DEPTH,            reverbParams.flEchoDepth);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_MODULATION_TIME,       reverbParams.flModulationTime);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_MODULATION_DEPTH,      reverbParams.flModulationDepth);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverbParams.flAirAbsorptionGainHF);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_HFREFERENCE,           reverbParams.flHFReference);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_LFREFERENCE,           reverbParams.flLFReference);
	m_fx.alEffectf (m_effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,   reverbParams.flRoomRolloffFactor);
	m_fx.alEffecti (m_effect, AL_EAXREVERB_DECAY_HFLIMIT,         reverbParams.iDecayHFLimit);
	
	errorCode = alGetError();
	if (errorCode != AL_NO_ERROR)
	{
		TT_PANIC("Could not set reverb effect parameters.\n"
		         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
		return false;
	}
	
	// Load effect into auxiliary effect slot
	m_fx.alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, m_effect);
	errorCode = alGetError();
	if (errorCode != AL_NO_ERROR)
	{
		TT_PANIC("Could not load reverb effect into auxiliary effect slot.\n"
		         "OpenAL error %d: '%s'", errorCode, getALErrorMessage(errorCode));
		return false;
	}
	
	/*
	// Use this to disable reverb for a voice (or pass m_effectSlot to enable):
	//alSource3i(uiSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
	// */
	
#else
	// Unsupported platform
	TT_WARN("Reverb not supported on this platform.");
#endif
	
	m_activeReverbPreset = p_preset;
	
	return true;
}


// Buffer creation functions

bool OpenALSoundSystem::openBuffer(const BufferPtr& p_buffer)
{
	TT_NULL_ASSERT(p_buffer);
	if (p_buffer->getData() != 0)
	{
		TT_PANIC("Buffer already open.");
		return false;
	}
	
	BufferData* data = new BufferData;
	if (data->allocate() == false)
	{
		delete data;
		return false;
	}
	
	p_buffer->setData(data);
	
	return true;
}


bool OpenALSoundSystem::closeBuffer(Buffer* p_buffer)
{
	if (p_buffer == 0 || p_buffer->getData() == 0)
	{
		// Buffer data already cleaned up, most likely openBuffer() failed
		// This is a legal situation
		return true;
	}
	
	BufferData* data = reinterpret_cast<BufferData*>(p_buffer->getData());
	
	bool success = true;
	if (data->free() == false)
	{
		TT_PANIC("Failed to close buffer.");
		success = false;
		// we'll need to clean up the internal data to prevent more leaks.
	}
	
	delete data;
	p_buffer->setData(0);
	
	return success;
}


// Buffer Parameter functions

bool OpenALSoundSystem::setBufferData(const BufferPtr& p_buffer,
                                      const void*      p_data,
                                      size_type        p_frames,
                                      size_type        p_channels,
                                      size_type        p_sampleSize,
                                      size_type        p_sampleRate,
                                      bool             p_ownership)
{
	if (p_channels != 1 && p_channels != 2)
	{
		TT_PANIC("Unsupported channel count %d", p_channels);
		return false;
	}
	
	if (p_sampleSize != 8 && p_sampleSize != 16)
	{
		TT_PANIC("Unsupported sample size %d", p_sampleSize);
		return false;
	}
	
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->setData(p_data, p_frames, p_channels, p_sampleSize, p_sampleRate, p_ownership) : false;
}


size_type OpenALSoundSystem::getBufferLength(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getFrameCount() : 0;
}


size_type OpenALSoundSystem::getBufferChannelCount(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getChannelCount() : 0;
}


size_type OpenALSoundSystem::getBufferSampleSize(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getSampleSize() : 0;
}


size_type OpenALSoundSystem::getBufferSampleRate(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getSampleRate() : 0;
}


// Stream Functions

bool OpenALSoundSystem::openStream(const StreamPtr& p_stream)
{
	TT_NULL_ASSERT(p_stream);
	if (p_stream->getData() != 0)
	{
		TT_PANIC("Stream already open.");
		return false;
	}
	
	StreamSource* streamSource = p_stream->getSource();
	if (streamSource == 0)
	{
		TT_PANIC("Stream has no streamSource.");
		return false;
	}
	
	StreamData* data = new StreamData(streamSource);
	
	p_stream->setData(data);
	
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	const ALuint alSource = allocateSource();
	if (isValidSource(alSource) == false)
	{
		TT_PANIC("allocateSource Failed.");
		return false;
	}
	
	if (data->allocate(alSource, streamSource) == false)
	{
		return false;
	}
	
	if (m_updateStreamsInThread)
	{
		thread::CriticalSection criticalSection(&m_decodeMutex);
		m_streamsToAdd.push_back(p_stream.get());
	}
	
	return true;
}


bool OpenALSoundSystem::closeStream(Stream* p_stream)
{
	if (p_stream == 0 || p_stream->getData() == 0)
	{
		// Stream data already cleaned up, most likely openStream() failed
		// This is a legal situation
		return true;
	}
	
	if (m_updateStreamsInThread)
	{
		{
			thread::CriticalSection criticalSection(&m_decodeMutex);
			m_streamToRemove = p_stream;
		}
		
		// Block until stream is closed (so that we don't try to reference a possibly invalid StreamSource)
		m_decodeSemaphore.wait();
	}
	
	StreamData* data = static_cast<StreamData*>(p_stream->getData());
	
	const ALuint alSource = data->getALSource();
	
	data->free();
	
	freeSource(alSource);
	
	delete data;
	p_stream->setData(0);
	
	return true;
}


bool OpenALSoundSystem::playStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->play() : false;
}


bool OpenALSoundSystem::stopStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->stop() : false;
}


bool OpenALSoundSystem::pauseStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->pause() : false;
}


bool OpenALSoundSystem::resumeStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->resume() : false;
}


bool OpenALSoundSystem::updateStream(const StreamPtr& p_stream)
{
	if (m_updateStreamsInThread)
	{
		return true;
	}
	
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->update(p_stream->getSource()) : false;
}


bool OpenALSoundSystem::isStreamPlaying(const StreamPtr& p_stream)
{
	const StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->isPlaying() : false;
}


bool OpenALSoundSystem::isStreamPaused(const StreamPtr& p_stream)
{
	const StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->isPaused() : false;
}


real OpenALSoundSystem::getStreamVolumeRatio(const StreamPtr& p_stream)
{
	const StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->getVolumeRatio() : 0.0f;
}


bool OpenALSoundSystem::setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->setVolumeRatio(p_volumeRatio) : false;
}


real OpenALSoundSystem::getStreamVolume(const StreamPtr& p_stream)
{
	const StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->getVolumeDB() : 0.0f;
}


bool OpenALSoundSystem::setStreamVolume(const StreamPtr& p_stream, real p_volume)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->setVolumeDB(p_volume) : false;
}


bool OpenALSoundSystem::set3DAudioEnabled(bool p_enabled)
{
	if(p_enabled)
	{
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	}

	return SoundSystem::set3DAudioEnabled(p_enabled);
}


bool OpenALSoundSystem::setListenerPosition(const math::Vector3& p_position)
{
	if(is3DAudioEnabled() == false)
	{
		TT_PANIC("3D Audio is not enabled, call set3DAudioEnabled() first!");
		return false;
	}
	
	m_listenerPosition = p_position;
	alListener3f(AL_POSITION, m_listenerPosition.x, m_listenerPosition.y, m_listenerPosition.z);
	
	// TODO: Expose this
	static const ALfloat listenerOrientation[] = {0,0,-1, 0,1,0};
	
	alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, listenerOrientation);
	//alListenerf(AL_GAIN, 1.0f);
	
	return (checkALError() == false);
}


const math::Vector3& OpenALSoundSystem::getListenerPosition() const
{
	return m_listenerPosition;
}


bool OpenALSoundSystem::setPositionalAudioModel(const VoicePtr& p_voice, const audio::xact::RPCCurve* p_curve)
{
	VoiceData* data = getVoiceData(p_voice);
	if (data == 0) return false;
	
	using audio::xact::RPCPoint;

	const RPCPoint& first = p_curve->getFirstPoint();
	const RPCPoint& last  = p_curve->getLastPoint();

	switch(first.type)
	{
		case audio::xact::CurveType_Linear:
		{
			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

			data->setDistanceProperties(first.y, last.y, 1.0f);

			break;
		}

		default:
			TT_PANIC("Curve type (%d) not supported!", first.type);
	}

	return (checkALError() == false);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

OpenALSoundSystem::FxExtension::FxExtension()
:
supported(false)
#if defined(TT_FORCE_OPENAL_SOFT)
#elif defined(TT_PLATFORM_OSX_MAC)
,
alcASASetListener(0),
alcASAGetListener(0),
alcASASetSource(0),
alcASAGetSource(0),
enumALC_ASA_REVERB_ON(AL_NONE),
enumALC_ASA_REVERB_QUALITY(AL_NONE),
enumALC_ASA_REVERB_ROOM_TYPE(AL_NONE),
enumALC_ASA_REVERB_SEND_LEVEL(AL_NONE),
enumALC_ASA_REVERB_GLOBAL_LEVEL(AL_NONE)
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
,
alGenEffects(0),
alDeleteEffects(0),
alIsEffect(0),
alEffecti(0),
alEffectiv(0),
alEffectf(0),
alEffectfv(0),
alGetEffecti(0),
alGetEffectiv(0),
alGetEffectf(0),
alGetEffectfv(0),
alGenFilters(0),
alDeleteFilters(0),
alIsFilter(0),
alFilteri(0),
alFilteriv(0),
alFilterf(0),
alFilterfv(0),
alGetFilteri(0),
alGetFilteriv(0),
alGetFilterf(0),
alGetFilterfv(0),
alGenAuxiliaryEffectSlots(0),
alDeleteAuxiliaryEffectSlots(0),
alIsAuxiliaryEffectSlot(0),
alAuxiliaryEffectSloti(0),
alAuxiliaryEffectSlotiv(0),
alAuxiliaryEffectSlotf(0),
alAuxiliaryEffectSlotfv(0),
alGetAuxiliaryEffectSloti(0),
alGetAuxiliaryEffectSlotiv(0),
alGetAuxiliaryEffectSlotf(0),
alGetAuxiliaryEffectSlotfv(0)
#endif
{
}


void OpenALSoundSystem::FxExtension::init(ALCdevice* p_device)
{
	*this = FxExtension();  // reset to defaults first (default to unsupported)
	
	TT_NULL_ASSERT(p_device);
	if (p_device == 0)
	{
		return;
	}
#if defined(TT_FORCE_OPENAL_SOFT)	
#elif defined(TT_PLATFORM_OSX_MAC)
	
	if (alcIsExtensionPresent(p_device, alcstr("ALC_EXT_ASA")) == ALC_FALSE)
	{
		// ASA not supported (so leave the default values)
		TT_Printf("OpenAL Mac OS X ASA extension not supported on this system.\n");
		return;
	}
	
	// Load the extension function pointers
	alcASASetListener = (alcASASetListenerProcPtr)alcGetProcAddress(p_device, alcstr("alcASASetListener"));
	alcASAGetListener = (alcASAGetListenerProcPtr)alcGetProcAddress(p_device, alcstr("alcASAGetListener"));
	alcASASetSource   = (alcASASetSourceProcPtr)  alcGetProcAddress(p_device, alcstr("alcASASetSource"));
	alcASAGetSource   = (alcASAGetSourceProcPtr)  alcGetProcAddress(p_device, alcstr("alcASAGetSource"));
	
	// Get the extension enum values
	enumALC_ASA_REVERB_ON           = alcGetEnumValue(p_device, alcstr("ALC_ASA_REVERB_ON"));
	enumALC_ASA_REVERB_QUALITY      = alcGetEnumValue(p_device, alcstr("ALC_ASA_REVERB_QUALITY"));
	enumALC_ASA_REVERB_ROOM_TYPE    = alcGetEnumValue(p_device, alcstr("ALC_ASA_REVERB_ROOM_TYPE"));
	enumALC_ASA_REVERB_SEND_LEVEL   = alcGetEnumValue(p_device, alcstr("ALC_ASA_REVERB_SEND_LEVEL"));
	enumALC_ASA_REVERB_GLOBAL_LEVEL = alcGetEnumValue(p_device, alcstr("ALC_ASA_REVERB_GLOBAL_LEVEL"));
	
	// If any of the function pointers or enums could not be loaded, consider ASA to be unsupported
	if (alcGetError(p_device) != ALC_NO_ERROR      ||
	    alcASASetListener               == 0       ||
	    alcASAGetListener               == 0       ||
	    alcASASetSource                 == 0       ||
	    alcASAGetSource                 == 0       ||
	    enumALC_ASA_REVERB_ON           == AL_NONE ||
	    enumALC_ASA_REVERB_QUALITY      == AL_NONE ||
	    enumALC_ASA_REVERB_ROOM_TYPE    == AL_NONE ||
	    enumALC_ASA_REVERB_SEND_LEVEL   == AL_NONE ||
	    enumALC_ASA_REVERB_GLOBAL_LEVEL == AL_NONE)
	{
		// Reset to the default, unsupported, state
		TT_Printf("This system claims to support the OpenAL Mac OS X ASA extension, but not all required functions are present.\n");
		*this = FxExtension();
	}
	else
	{
		TT_Printf("OpenAL Mac OS X ASA extension is fully supported on this system.\n");
		supported = true;
	}
	
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
	
	if (alcIsExtensionPresent(p_device, (ALCchar*)ALC_EXT_EFX_NAME) == ALC_FALSE)
	{
		// EFX not supported (so leave the default values)
		TT_Printf("OpenAL EFX extension not supported on this system.\n");
		return;
	}
	
	// Load the extension function pointers
	alGenEffects                 = (LPALGENEFFECTS)                alGetProcAddress("alGenEffects");
	alDeleteEffects              = (LPALDELETEEFFECTS)             alGetProcAddress("alDeleteEffects");
	alIsEffect                   = (LPALISEFFECT)                  alGetProcAddress("alIsEffect");
	alEffecti                    = (LPALEFFECTI)                   alGetProcAddress("alEffecti");
	alEffectiv                   = (LPALEFFECTIV)                  alGetProcAddress("alEffectiv");
	alEffectf                    = (LPALEFFECTF)                   alGetProcAddress("alEffectf");
	alEffectfv                   = (LPALEFFECTFV)                  alGetProcAddress("alEffectfv");
	alGetEffecti                 = (LPALGETEFFECTI)                alGetProcAddress("alGetEffecti");
	alGetEffectiv                = (LPALGETEFFECTIV)               alGetProcAddress("alGetEffectiv");
	alGetEffectf                 = (LPALGETEFFECTF)                alGetProcAddress("alGetEffectf");
	alGetEffectfv                = (LPALGETEFFECTFV)               alGetProcAddress("alGetEffectfv");
	alGenFilters                 = (LPALGENFILTERS)                alGetProcAddress("alGenFilters");
	alDeleteFilters              = (LPALDELETEFILTERS)             alGetProcAddress("alDeleteFilters");
	alIsFilter                   = (LPALISFILTER)                  alGetProcAddress("alIsFilter");
	alFilteri                    = (LPALFILTERI)                   alGetProcAddress("alFilteri");
	alFilteriv                   = (LPALFILTERIV)                  alGetProcAddress("alFilteriv");
	alFilterf                    = (LPALFILTERF)                   alGetProcAddress("alFilterf");
	alFilterfv                   = (LPALFILTERFV)                  alGetProcAddress("alFilterfv");
	alGetFilteri                 = (LPALGETFILTERI)                alGetProcAddress("alGetFilteri");
	alGetFilteriv                = (LPALGETFILTERIV)               alGetProcAddress("alGetFilteriv");
	alGetFilterf                 = (LPALGETFILTERF)                alGetProcAddress("alGetFilterf");
	alGetFilterfv                = (LPALGETFILTERFV)               alGetProcAddress("alGetFilterfv");
	alGenAuxiliaryEffectSlots    = (LPALGENAUXILIARYEFFECTSLOTS)   alGetProcAddress("alGenAuxiliaryEffectSlots");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
	alIsAuxiliaryEffectSlot      = (LPALISAUXILIARYEFFECTSLOT)     alGetProcAddress("alIsAuxiliaryEffectSlot");
	alAuxiliaryEffectSloti       = (LPALAUXILIARYEFFECTSLOTI)      alGetProcAddress("alAuxiliaryEffectSloti");
	alAuxiliaryEffectSlotiv      = (LPALAUXILIARYEFFECTSLOTIV)     alGetProcAddress("alAuxiliaryEffectSlotiv");
	alAuxiliaryEffectSlotf       = (LPALAUXILIARYEFFECTSLOTF)      alGetProcAddress("alAuxiliaryEffectSlotf");
	alAuxiliaryEffectSlotfv      = (LPALAUXILIARYEFFECTSLOTFV)     alGetProcAddress("alAuxiliaryEffectSlotfv");
	alGetAuxiliaryEffectSloti    = (LPALGETAUXILIARYEFFECTSLOTI)   alGetProcAddress("alGetAuxiliaryEffectSloti");
	alGetAuxiliaryEffectSlotiv   = (LPALGETAUXILIARYEFFECTSLOTIV)  alGetProcAddress("alGetAuxiliaryEffectSlotiv");
	alGetAuxiliaryEffectSlotf    = (LPALGETAUXILIARYEFFECTSLOTF)   alGetProcAddress("alGetAuxiliaryEffectSlotf");
	alGetAuxiliaryEffectSlotfv   = (LPALGETAUXILIARYEFFECTSLOTFV)  alGetProcAddress("alGetAuxiliaryEffectSlotfv");
	
	// If any of the function pointers could not be loaded, consider EFX to be unsupported
	if (alGenEffects                 == 0 ||
	    alDeleteEffects              == 0 ||
	    alIsEffect                   == 0 ||
	    alEffecti                    == 0 ||
	    alEffectiv                   == 0 ||
	    alEffectf                    == 0 ||
	    alEffectfv                   == 0 ||
	    alGetEffecti                 == 0 ||
	    alGetEffectiv                == 0 ||
	    alGetEffectf                 == 0 ||
	    alGetEffectfv                == 0 ||
	    alGenFilters                 == 0 ||
	    alDeleteFilters              == 0 ||
	    alIsFilter                   == 0 ||
	    alFilteri                    == 0 ||
	    alFilteriv                   == 0 ||
	    alFilterf                    == 0 ||
	    alFilterfv                   == 0 ||
	    alGetFilteri                 == 0 ||
	    alGetFilteriv                == 0 ||
	    alGetFilterf                 == 0 ||
	    alGetFilterfv                == 0 ||
	    alGenAuxiliaryEffectSlots    == 0 ||
	    alDeleteAuxiliaryEffectSlots == 0 ||
	    alIsAuxiliaryEffectSlot      == 0 ||
	    alAuxiliaryEffectSloti       == 0 ||
	    alAuxiliaryEffectSlotiv      == 0 ||
	    alAuxiliaryEffectSlotf       == 0 ||
	    alAuxiliaryEffectSlotfv      == 0 ||
	    alGetAuxiliaryEffectSloti    == 0 ||
	    alGetAuxiliaryEffectSlotiv   == 0 ||
	    alGetAuxiliaryEffectSlotf    == 0 ||
	    alGetAuxiliaryEffectSlotfv   == 0)
	{
		// Reset to the default, unsupported, state
		TT_Printf("This system claims to support the OpenAL EFX extension, but not all required functions are present.\n");
		*this = FxExtension();
	}
	else
	{
		TT_Printf("OpenAL EFX extension is fully supported on this system.\n");
		supported = true;
	}
	
#endif
}


OpenALSoundSystem::OpenALSoundSystem(identifier p_identifier, bool p_updateStreamsInThread)
:
SoundSystem(p_identifier),
m_device(0),
m_context(0),
#if defined(OPENAL_RESOURCE_LEAK)
m_sources(0),
#endif
m_listenerPosition(),
m_activeReverbPreset(ReverbPreset_None),
m_fx(),
#if defined(TT_FORCE_OPENAL_SOFT)	
#elif defined(TT_PLATFORM_OSX_MAC)
// No ASA members
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
m_effectSlot(AL_EFFECTSLOT_NULL),
m_effect(AL_EFFECT_NULL),
#endif
m_updateStreamsInThread(p_updateStreamsInThread),
m_decodeThread(),
m_decodeThreadShouldExit(false),
m_streamsToAdd(),
m_activeStreams(),
m_streamToRemove(0),
m_decodeMutex(),
m_decodeSemaphore()
{
#if !defined(TT_PLATFORM_LNX) && !defined(TT_FORCE_OPENAL_SOFT)
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
#endif

	// initialize with default device
	m_device = alcOpenDevice(NULL);
	if (m_device == 0)
	{
		TT_PANIC("Unable to open default device.");
		return;
	}
	
	m_context = alcCreateContext(m_device, NULL);
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcCreateContext error code %d: '%s'", error, getALCErrorMessage(error));
		break;
	}
	
	alcMakeContextCurrent(m_context);
	switch (ALCenum error = alcGetError(m_device))
	{
	case ALC_NO_ERROR:
		break;
		
	default:
		TT_PANIC("alcMakeContextCurrent error code %d: '%s'", error, getALCErrorMessage(error));
		break;
	}

#ifndef TT_BUILD_FINAL
	// Check available hardware resource
	ALCint attributesSize(0);
	alcGetIntegerv(m_device, ALC_ATTRIBUTES_SIZE, 1, &attributesSize);

	std::vector<ALCint> attributes(attributesSize);
	alcGetIntegerv(m_device, ALC_ALL_ATTRIBUTES, attributesSize, &attributes[0]);

	for(std::vector<ALCint>::iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		if( (*it) == ALC_MONO_SOURCES )
		{
			// Count is in next location
			++it;
			TT_Printf("[OpenAL] Max mono sources: %d\n", (*it));
		}
		else if( (*it) == ALC_STEREO_SOURCES )
		{
			// Count is in next location
			++it;
			TT_Printf("[OpenAL] Max stereo sources: %d\n", (*it));
		}
	}
	
	const char* alExtensions = reinterpret_cast<const char*>(alGetString(AL_EXTENSIONS));
	if (alExtensions != 0)
	{
		TT_Printf("[OpenAL] Extensions: %s\n", alExtensions);
	}
	const char* alcExtensions = reinterpret_cast<const char*>(alcGetString(m_device, ALC_EXTENSIONS));
	if (alcExtensions != 0)
	{
		TT_Printf("[OpenAL] ALC extensions: %s\n", alcExtensions);
	}
	
#endif // TT_BUILD_FINAL
	
	// Initialize the OpenAL audio effects extension (if supported)
	m_fx.init(m_device);
	
	allocateSources();
	
	TT_ASSERTMSG(checkALError() == false, "Error during construction of OpenALSoundSystem");
	
	
	if (m_updateStreamsInThread)
	{
		m_decodeSemaphore.create(0);
		if (m_decodeSemaphore.isValid() == false)
		{
			TT_PANIC("Failed to create a semaphore for stream decoding thread communication.");
			return;
		}
		
		m_decodeThreadShouldExit = false;
		m_decodeThread = thread::create(staticDecodeStreamsThread, this, false, 0, tt::thread::priority_normal,
				tt::thread::Affinity_None, "Audio Decoder Thread");
		if (m_decodeThread == 0)
		{
			TT_PANIC("Failed to create the stream decoding thread.");
			return;
		}
	}
}


bool OpenALSoundSystem::allocateSources()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
#if defined(OPENAL_RESOURCE_LEAK)
	m_sources = new OpenALSource[SourceCount];
	for (int i = 0; i < SourceCount; ++i)
	{
		if (m_sources[i].source != static_cast<ALuint>(AL_INVALID))
		{
			continue;
		}
		alGenSources(1, &(m_sources[i].source));

		if(checkALError()) return false;
	}
#endif //defined(OPENAL_RESOURCE_LEAK)
	return true;
}


bool OpenALSoundSystem::freeSources()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
#if defined(OPENAL_RESOURCE_LEAK)
	for (int i = 0; i < SourceCount; ++i)
	{
		TT_ASSERTMSG(m_sources[i].used == false, "Source %d still in use.", i);
		if (m_sources[i].source == static_cast<ALuint>(AL_INVALID))
		{
			continue;
		}
		
		alDeleteSources(1, &(m_sources[i].source));
		m_sources[i].source = static_cast<ALuint>(AL_INVALID);
		
		if(checkALError()) return false;
	}
	delete[] m_sources;
	m_sources = 0;
#endif //defined(OPENAL_RESOURCE_LEAK)
	return true;
}


#if !defined(TT_BUILD_FINAL) && !defined(OPENAL_RESOURCE_LEAK)
static s32 gs_sourceCount = 0;
#endif // #if !defined(TT_BUILD_FINAL)


ALuint OpenALSoundSystem::allocateSource()
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
#if defined(OPENAL_RESOURCE_LEAK)
	static int lastIndex = 0;
	for (int i = lastIndex + 1; i != lastIndex; ++i)
	{
		if  (i >= SourceCount)
		{
			i = -1; // -1 so after the ++i it's 0.
		}
		
		
		if (m_sources[i].used == false &&
		    isValidSource(m_sources[i].source))
		{
			{
				ALenum state;
				
				alGetSourcei(m_sources[i].source, AL_SOURCE_STATE, &state);
				switch (ALenum error = alGetError())
				{
					case AL_NO_ERROR:
						break;
						
					default:
						TT_PANIC("alGetSourcei: %s", getALErrorMessage(error));
						return static_cast<ALuint>(AL_INVALID);
				}
				
				if (state != AL_INITIAL)
				{
					{// Start debug
						
						//TT_Printf("Source [%d] %d didn't have initial state (was used?). Checking attributes.\n",
						//          i, s32(m_sources[i].source));
						ALenum buffer = AL_INVALID;
						
						alGetSourcei(m_sources[i].source, AL_BUFFER, &state);
						switch (ALenum error = alGetError())
						{
							case AL_NO_ERROR:
								break;
								
							default:
								TT_PANIC("alGetSourcei: %s", getALErrorMessage(error));
								return static_cast<ALuint>(AL_INVALID);
						}
						// FIXME: Change the TT_WARNING below back to a TT_ASSERTMSG
						// FIXME: And on iphone it seems that buffer gets the value AL_INVALID after setting it to AL_NONE.
						TT_WARNING(buffer == AL_NONE || buffer == AL_INVALID, "AL_BUFFER should be AL_NONE (0x%x) but it's: 0x%x", s32(AL_NONE), u32(buffer));
						
						// check AL_LOOPING = AL_FALSE
						ALint looping = getALSourcei(m_sources[i].source, AL_LOOPING);
						TT_ASSERTMSG(looping == AL_FALSE, "AL_LOOPING should be AL_FALSE");
						
						// check AL_BUFFERS_QUEUED = 0
						ALint queued = getALSourcei(m_sources[i].source, AL_BUFFERS_QUEUED);
						TT_ASSERTMSG(queued == 0, "AL_BUFFERS_QUEUED should be 0 but is: %d.", s32(queued));
						
						// check AL_BUFFERS_PROCESSED = 0
						ALint processed = getALSourcei(m_sources[i].source, AL_BUFFERS_PROCESSED);
						TT_ASSERTMSG(processed == 0, "AL_BUFFERS_PROCESSED should be 0 but is: %d.", s32(processed));
						
						// check AL_SOURCE_TYPE == AL_UNDETERMINED
						ALint sourceType = getALSourcei(m_sources[i].source, AL_SOURCE_TYPE);
						TT_ASSERTMSG(sourceType == AL_UNDETERMINED, "AL_SOURCE_TYPE should be AL_UNDETERMINED but is: 0x%x", u32(sourceType));
						
						
						// Report part done, now repair.
						if (buffer != AL_NONE)
						{
							alSourcei(m_sources[i].source, AL_BUFFER, AL_NONE);
							TT_ASSERT(alGetError() == AL_NO_ERROR);
							ALint newbuffer = getALSourcei(m_sources[i].source, AL_BUFFER);
							TT_ASSERT(newbuffer == AL_NONE);
							// FIXME: On iphone it seems that buffer get's set to AL_INVALID (-1). Added this to stop the debug flood.
							if (buffer != AL_INVALID)
							{
								TT_Printf("REPAIR: Setting buffer to none for source [%d] %d should be done!\n", i, s32(m_sources[i].source));
							}
						}
						
						if (looping != AL_FALSE)
						{
							alSourcei(m_sources[i].source, AL_LOOPING, AL_FALSE);
							TT_ASSERT(alGetError() == AL_NO_ERROR);
							looping = getALSourcei(m_sources[i].source, AL_LOOPING);
							TT_ASSERTMSG(looping == AL_FALSE, "AL_LOOPING should be AL_FALSE");
							TT_Printf("REPAIR: Setting AL_LOOPING to AL_FALSE for source [%d] %d should be done!\n", i, s32(m_sources[i].source));
						}
						
						// Try next and run to all the checks again.
						//continue;
					}// End debug
					
					
					
					// Because of a bug in the implementation of openal rewind doesn't return the source state to initial.
					// Also accept stopped as state.
					// FIXME: debug test: for some reason state because 0, not sure what that is.
					if (state != AL_STOPPED && state != 0) // FIXME: Remove != 0 check!
					{
						TT_WARN("Found source: [%d] %d which didn't have the initial state or AL_STOPPED. (Found state: 0x%x)",
								i, s32(m_sources[i].source), u32(state));
						
						// Force another rewind.
						alSourceStop(m_sources[i].source);
						alSourceRewind(m_sources[i].source);
						switch (ALenum error = alGetError())
						{
							case AL_NO_ERROR:
								(void)error;
								break;
								
							default:
								TT_PANIC("alSourceRewind failed (%d): '%s'", error, getALErrorMessage(error));
								break;
						}
						continue;
					}
				}
			}
			
			m_sources[i].used = true;
			
			//TT_Printf("OpenALSoundSystem::allocateSource - returning [%d] source: %d\n", i, s32(m_sources[i].source));
			// DEBUG! Remove this define check!
#if defined(TT_BUILD_FINAL)
			lastIndex = i;
#endif
			return m_sources[i].source;
		}
	}
	TT_PANIC("No sources left.");
	return static_cast<ALuint>(AL_INVALID);
#else //defined(OPENAL_RESOURCE_LEAK)
	ALuint ret = static_cast<ALuint>(AL_INVALID);
	alGenSources(1, &ret);
	
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR:
#if !defined(TT_BUILD_FINAL)
		++gs_sourceCount;
#endif // #if !defined(TT_BUILD_FINAL)
		(void)error;
		break;
		
	default:
#if !defined(TT_BUILD_FINAL)
		TT_PANIC("OpenALSoundSystem::allocateSource Failed! Current number of source: %d.\n"
		         "The function: alGenSources failed (%d): '%s'.", 
		         gs_sourceCount, error, getALErrorMessage(error));
#endif //#if !defined(TT_BUILD_FINAL)
		return static_cast<ALuint>(AL_INVALID);
	}
	
	//TT_Printf("OpenALSoundSystem::allocateSource - returning source: %d\n", s32(ret));
	return ret;
#endif
}


void OpenALSoundSystem::freeSource(ALuint p_source)
{
	TT_ASSERTMSG(checkALError() == false, "Found pre-existing error state!");
	
	//TT_Printf("OpenALSoundSystem::freeSource - source: %d\n", s32(p_source));
#if defined(OPENAL_RESOURCE_LEAK)
	if (p_source == static_cast<ALuint>(AL_INVALID))
	{
		return;
	}
	
	for (int i = 0; i < SourceCount; ++i)
	{
		if (m_sources[i].source == p_source)
		{
			//TT_Printf("OpenALSoundSystem::freeSource - found it source: [%d] %d. rewinding and setting AL_BUFFER to AL_NONE, AL_LOOPING to AL_FALSE!\n",
			//          i, s32(m_sources[i].source));
			TT_ASSERT(m_sources[i].used);
			alSourceRewind(m_sources[i].source);
			switch (ALenum error = alGetError())
			{
			case AL_NO_ERROR:
				(void)error;
				break;
				
			default:
				TT_PANIC("alSourceRewind failed (%d): '%s'", error, getALErrorMessage(error));
				break;
			}
			
			alSourcei(m_sources[i].source, AL_BUFFER, AL_NONE);
			switch (ALenum error = alGetError())
			{
			case AL_NO_ERROR:
				(void)error;
				break;
				
			default:
				TT_PANIC("alSourcei (AL_BUFFER) failed (%d): '%s'", error, getALErrorMessage(error));
				break;
			}
			
			alSourcei(m_sources[i].source, AL_LOOPING, AL_FALSE);
			switch (ALenum error = alGetError())
			{
			case AL_NO_ERROR:
				(void)error;
				break;
				
			default:
				TT_PANIC("alSourcei (AL_LOOPING) failed (%d): '%s'", error, getALErrorMessage(error));
				break;
			}
			
			m_sources[i].used = false;
			return;
		}
	}
	TT_PANIC("Couldn't find source: %d!", s32(p_source));
#else
	alDeleteSources(1, &p_source);
	
	switch (ALenum error = alGetError())
	{
	case AL_NO_ERROR:
		(void)error;
#if !defined(TT_BUILD_FINAL)
		--gs_sourceCount;
		TT_ASSERT(gs_sourceCount >= 0);
#endif // #if !defined(TT_BUILD_FINAL)
		break;
		
	default:
		TT_PANIC("OpenALSoundSystem::freeSource Failed!\n"
		         "The function: alDeleteSources failed (%d): '%s'", error, getALErrorMessage(error));
		return;
	}
	
#endif
}


OpenALSoundSystem::VoiceData* OpenALSoundSystem::getVoiceData(const VoicePtr&  p_voice)
{
	return checkVoiceOpen(p_voice) ? reinterpret_cast<VoiceData*>(p_voice->getData()) : 0;
}


OpenALSoundSystem::BufferData* OpenALSoundSystem::getBufferData(const BufferPtr& p_buffer)
{
	return checkBufferOpen(p_buffer) ? reinterpret_cast<BufferData*>(p_buffer->getData()) : 0;
}


OpenALSoundSystem::StreamData* OpenALSoundSystem::getStreamData(const StreamPtr& p_stream)
{
	return checkStreamOpen(p_stream) ? reinterpret_cast<StreamData*>(p_stream->getData()) : 0;
}


int OpenALSoundSystem::staticDecodeStreamsThread(void* p_soundSystem)
{
	TT_NULL_ASSERT(p_soundSystem);
	static_cast<OpenALSoundSystem*>(p_soundSystem)->decodeStreams();
	return 0;
}


void OpenALSoundSystem::decodeStreams()
{
	if (m_updateStreamsInThread == false)
	{
		TT_PANIC("Should not create a stream update/decode thread if streams "
		         "aren't supposed to be updated in a separate thread.");
		return;
	}
	
#if !defined(TT_BUILD_FINAL)
#define SHOW_STREAM_TIMINGS 0  // set to 1 to enable timing of stream updates
#else
#define SHOW_STREAM_TIMINGS 0
#endif
	
	system::Time* tm = system::Time::getInstance();
	
	for ( ;; )
	{
		// Block if audio system was suspended
		//OSWaitEvent(&m_audioSuspendEvent);  // FIXME: Does OpenAL need something like this too?
		
		// Check for new streams
		{
			thread::CriticalSection criticalSection(&m_decodeMutex);
			
			if (m_decodeThreadShouldExit)
			{
				TT_Printf("OpenALSoundSystem::decodeStreams: Someone wants me to leave... bye bye!\n");
				break;
			}
			
			if (m_streamsToAdd.empty() == false)
			{
				m_activeStreams.insert(
					m_activeStreams.end(), m_streamsToAdd.begin(), m_streamsToAdd.end());
				m_streamsToAdd.clear();
			}
		}
		
		// Update active streams
		const u64 updateStart = tm->getMilliSeconds();
		u32 shortestBufferInMs = 60000;
		for (StreamList::iterator it = m_activeStreams.begin(); it != m_activeStreams.end(); ++it)
		{
			Stream* stream = (*it);
			TT_NULL_ASSERT(stream);
			
			StreamData* data = reinterpret_cast<StreamData*>(stream->getData());
			TT_NULL_ASSERT(data);
			
			if (data->isPlaying())
			{
				if (data->update(stream->getSource()) == false)
				{
					// NOTE: update() call will have already panicked with more detailed information
					//TT_PANIC("stream update failed");
				}
			}
			
			if (data->getBufferLengthInMs() > 0 &&
			    data->getBufferLengthInMs() < shortestBufferInMs)
			{
				shortestBufferInMs = data->getBufferLengthInMs();
			}
		}
		
		const u32 updateMs = static_cast<u32>(tm->getMilliSeconds() - updateStart);
#if SHOW_STREAM_TIMINGS
		if (updateMs > 25 && m_activeStreams.empty() == false)
		{
			TT_Printf("OpenALSoundSystem::decodeStreams: Updating %u streams took %u ms.\n",
			          m_activeStreams.size(), updateMs);
		}
#endif
		
		// Check for stream to remove
		{
			thread::CriticalSection criticalSection(&m_decodeMutex);
			
			if (m_streamToRemove != 0)
			{
#if defined(TT_BUILD_DEV)
				// Make sure this stream is found!
				StreamList::const_iterator it = std::find(m_activeStreams.begin(), m_activeStreams.end(), m_streamToRemove);
				TT_ASSERT(it != m_activeStreams.end());
#endif
				m_activeStreams.remove(m_streamToRemove);
				m_streamToRemove = 0;
				
				m_decodeSemaphore.signal();
			}
		}
		
		// If we don't run the risk of missing buffer refills, sleep for a bit to give the CPU a rest
		static const u32 idleTimeInMs = 20;
		if ((updateMs + idleTimeInMs) < shortestBufferInMs)
		{
			thread::sleep(static_cast<int>(idleTimeInMs));
		}
	}
}

// Namespace end
}
}
