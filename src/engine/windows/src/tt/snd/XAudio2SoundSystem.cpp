#include <Windowsx.h>

#include <tt/app/ComHelper.h>
#include <tt/audio/helpers.h>
#include <tt/math/math.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/Buffer.h>
#include <tt/snd/snd.h>
#include <tt/snd/Stream.h>
#include <tt/snd/StreamSource.h>
#include <tt/snd/utils.h>
#include <tt/snd/Voice.h>
#include <tt/snd/XAudio2SoundSystem.h>
#include <tt/system/Time.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/thread.h>


namespace tt {
namespace snd {

//--------------------------------------------------------------------------------------------------
// Types

struct XAudio2SoundSystem::BufferData
{
	XAUDIO2_BUFFER buffer;
	WAVEFORMATEX   format;
	
	BufferData()
	{
		mem::zero8(&buffer, sizeof(XAUDIO2_BUFFER));
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		
		mem::zero8(&format, sizeof(WAVEFORMATEX));
		format.cbSize = sizeof(WAVEFORMATEX);
		
		free();
	}
	
	~BufferData()
	{
		free();
	}
	
	bool allocate(size_type p_size)
	{
		free();
		
		buffer.pAudioData = new BYTE[p_size];
		if (buffer.pAudioData == 0)
		{
			TT_PANIC("Failed to allocate %d bytes for audio buffer.", p_size);
			return false;
		}
		
		return true;
	}
	
	bool free()
	{
		if (buffer.pAudioData != 0)
		{
			delete[] buffer.pAudioData;
			buffer.pAudioData = 0;
		}
		
		mem::zero8(&buffer, sizeof(XAUDIO2_BUFFER));
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		
		mem::zero8(&format, sizeof(WAVEFORMATEX));
		format.cbSize = sizeof(WAVEFORMATEX);
		
		return true;
	}
	
	bool setData(const void* p_data,
	             size_type   p_frames,
	             size_type   p_channels,
	             size_type   p_sampleSize,
	             size_type   p_sampleRate,
	             bool        p_ownership)
	{
		free();
		
		// sanity checking on input
		if (p_channels <= 0 || p_channels > XAUDIO2_MAX_AUDIO_CHANNELS)
		{
			TT_PANIC("Invalid channel count %d, must be between 1 and %d.",
			         p_channels, XAUDIO2_MAX_AUDIO_CHANNELS);
			return false;
		}
		if (p_sampleRate < XAUDIO2_MIN_SAMPLE_RATE || p_sampleRate > XAUDIO2_MAX_SAMPLE_RATE)
		{
			TT_PANIC("Invalid sample rate %d, must be between %d and %d.",
			         p_sampleRate, XAUDIO2_MIN_SAMPLE_RATE, XAUDIO2_MAX_SAMPLE_RATE);
			return false;
		}
		if (p_channels > 2)
		{
			if (p_sampleSize % 8 != 0)
			{
				TT_PANIC("Invalid sample size %d, must be a multiple of 8.", p_sampleSize);
				return false;
			}
		}
		else
		{
			if (p_sampleSize != 8 && p_sampleSize != 16)
			{
				TT_PANIC("Invalid sample size %d, must be 8 or 16.", p_sampleSize);
				return false;
			}
		}
		
		// set up a format
		format.wFormatTag      = static_cast<WORD>((p_channels > 2) ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM);
		format.nChannels       = static_cast<WORD>(p_channels);
		format.nSamplesPerSec  = static_cast<DWORD>(p_sampleRate);
		format.nAvgBytesPerSec = static_cast<DWORD>(p_sampleRate * p_channels * (p_sampleSize / 8));
		format.nBlockAlign     = static_cast<WORD>(p_channels * (p_sampleSize / 8));
		format.wBitsPerSample  = static_cast<WORD>(p_sampleSize);
		format.cbSize          = static_cast<WORD>(sizeof(format));
		
		buffer.AudioBytes = static_cast<UINT32>(format.nBlockAlign * p_frames);
		buffer.PlayLength = static_cast<UINT32>(p_frames);
		
		if (p_ownership)
		{
			buffer.pAudioData = reinterpret_cast<BYTE*>(const_cast<void*>(p_data));
		}
		else
		{
			if (allocate(static_cast<size_type>(buffer.AudioBytes)) == false)
			{
				free();
				return false;
			}
			mem::copy8(const_cast<BYTE*>(buffer.pAudioData), p_data, static_cast<mem::size_type>(buffer.AudioBytes));
		}
		
		return true;
	}
	
	size_type getChannelCount()
	{
		return static_cast<size_type>(format.nChannels);
	}
	
	size_type getSampleSize()
	{
		return static_cast<size_type>(format.wBitsPerSample);
	}
	
	size_type getSampleRate()
	{
		return static_cast<size_type>(format.nSamplesPerSec);
	}
	
	size_type getFrameCount()
	{
		return static_cast<size_type>(buffer.PlayLength);
	}
};


struct XAudio2SoundSystem::VoiceData : public IXAudio2VoiceCallback
{
	IXAudio2SourceVoice*    voice;
	BufferPtr               buffer;
	bool                    paused;
	bool                    playing;
	
	VoiceData()
	:
	voice(0),
	paused(false),
	playing(false)
	{
		free();
	}
	
	~VoiceData()
	{
		free();
	}
	
	bool allocate()
	{
		free();
		
		// 
		
		
		return true;
	}
	
	bool free()
	{
		if (voice != 0)
		{
			voice->DestroyVoice();
			voice = 0;
		}
		
		playing = false;
		paused  = false;
		
		buffer.reset();
		
		return true;
	}
	
	
	bool isPlaying()
	{
		return playing;
	}
	
	bool isPaused()
	{
		return paused;
	}
	
	bool play(bool p_looping)
	{
		if (buffer == 0)
		{
			TT_WARN("Attempt to play voice without buffer.");
			return false;
		}
		
		if (isPlaying())
		{
			TT_WARN("Attempt to play voice that's already playing.");
			return false;
		}
		
		BufferData* data = reinterpret_cast<BufferData*>(buffer->getData());
		
		XAUDIO2_BUFFER bf = {0};
		mem::copy8(&bf, &data->buffer, sizeof(XAUDIO2_BUFFER));
		
		if (p_looping)
		{
			bf.LoopLength = bf.PlayLength;
			bf.LoopCount  = XAUDIO2_LOOP_INFINITE;
		}
		else
		{
			bf.LoopCount = 0;
		}
		
		HRESULT hr = voice->SubmitSourceBuffer(&bf, 0);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to submit buffer to voice: 0x%08X.", hr);
			return false;
		}
		
		hr = voice->Start(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to start voice: 0x%08X.", hr);
			voice->FlushSourceBuffers();
			return false;
		}
		playing = true;
		return true;
	}
	
	bool stop()
	{
		if (isPlaying() == false)
		{
			return true;
		}
		
		HRESULT hr = voice->Stop(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to stop voice: 0x%08X.", hr);
			return false;
		}
		voice->FlushSourceBuffers();
		return true;
	}
	
	bool pause()
	{
		if (isPlaying() == false)
		{
			TT_WARN("Attempt to pause a voice that's not playing.");
			return false;
		}
		
		if (isPaused())
		{
			return true;
		}
		
		HRESULT hr = voice->Stop(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to stop voice: 0x%08X.", hr);
			return false;
		}
		return true;
	}
	
	bool resume()
	{
		if (isPlaying() == false)
		{
			TT_WARN("Attempt to resume a voice that's not playing.");
			return false;
		}
		
		if (isPaused() == false)
		{
			return true;
		}
		
		HRESULT hr = voice->Start(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to start voice: 0x%08X.", hr);
			return false;
		}
		return true;
	}
	
	bool setBuffer(const BufferPtr& p_buffer, IXAudio2* p_engine)
	{
		if (isPlaying())
		{
			TT_WARN("Stop voice before assigning a new buffer.");
			return false;
		}
		
		free();
		
		if (p_buffer != 0)
		{
			BufferData* data = reinterpret_cast<BufferData*>(p_buffer->getData());
			HRESULT hr = p_engine->CreateSourceVoice(&voice, &data->format, 0,
			                                         XAUDIO2_DEFAULT_FREQ_RATIO,
			                                         this,
			                                         0,
			                                         0);
			if (FAILED(hr))
			{
				TT_PANIC("Failed to allocate voice.");
				return false;
			}
			buffer = p_buffer;
		}
		return true;
	}
	
	real getPlaybackRatio()
	{
		float ratio(0.0f);
		voice->GetFrequencyRatio(&ratio);
		return static_cast<real>(ratio);
	}
	
	bool setPlaybackRatio(real p_ratio)
	{
		HRESULT hr = voice->SetFrequencyRatio(static_cast<float>(p_ratio), XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to set playback ratio: 0x%08X.", hr);
			return false;
		}
		return true;
	}
	
	real getVolumeRatio()
	{
		float ratio(0.0f);
		voice->GetVolume(&ratio);
		return static_cast<real>(ratio);
	}
	
	bool setVolumeRatio(real p_ratio)
	{
		HRESULT hr = voice->SetVolume(static_cast<float>(p_ratio), XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to set volume ratio: 0x%08X.", hr);
			return false;
		}
		return true;
	}
	
	// IXAudio2VoiceCallback functions
	
	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32)
	{
	}
	
	STDMETHOD_(void, OnVoiceProcessingPassEnd)()
	{
	}
	
	STDMETHOD_(void, OnStreamEnd)()
	{
		playing = false;
	}
	
	STDMETHOD_(void, OnBufferStart)(void*)
	{
	}
	
	STDMETHOD_(void, OnBufferEnd)(void*)
	{
	}
	
	STDMETHOD_(void, OnLoopEnd)(void*)
	{
	}
	
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT)
	{
	}
};



// Stream types

enum { BufferCount = 2 };

struct XAudio2SoundSystem::StreamData : public IXAudio2VoiceCallback
{
	struct StreamBuffer
	{
		XAUDIO2_BUFFER buffer;
		bool           empty;
		
		StreamBuffer()
		:
		empty(true)
		{
			mem::zero8(&buffer, sizeof(buffer));
		}
	};
	
	IXAudio2SourceVoice* voice;
	StreamSource*        source;
	StreamBuffer*        buffers;
	bool                 paused;
	bool                 playing;
	bool                 hwPlaying;
	bool                 playbackDone;
	size_type            bufferCount;
	
	u32 bufferLengthInMs;  // used by the stream decoding thread to estimate whether it can idle
	UINT64 totalSamplesBuffered; // total number of samples that have been buffered (needed for notifications)
	UINT64 notificationSample;   // absolute play position to notify the stream source at (0 for no notification)
	
	
	StreamData()
	:
	voice(0),
	source(0),
	buffers(0),
	paused(false),
	playing(false),
	hwPlaying(false),
	playbackDone(false),
	bufferCount(0),
	bufferLengthInMs(0),
	totalSamplesBuffered(0),
	notificationSample(0)
	{
	}
	
	virtual ~StreamData()
	{
		free();
	}
	
	bool allocate(IXAudio2* p_engine, StreamSource* p_source)
	{
		free();
		
		// sanity checking on input
		if (p_source->getChannelCount() <= 0 || p_source->getChannelCount() > XAUDIO2_MAX_AUDIO_CHANNELS)
		{
			TT_PANIC("Invalid channel count %d, must be between 1 and %d.",
			         p_source->getChannelCount(), XAUDIO2_MAX_AUDIO_CHANNELS);
			return false;
		}
		if (p_source->getFramerate() < XAUDIO2_MIN_SAMPLE_RATE || p_source->getFramerate() > XAUDIO2_MAX_SAMPLE_RATE)
		{
			TT_PANIC("Invalid sample rate %d, must be between %d and %d.",
			         p_source->getFramerate(), XAUDIO2_MIN_SAMPLE_RATE, XAUDIO2_MAX_SAMPLE_RATE);
			return false;
		}
		if (p_source->getChannelCount() > 2)
		{
			if (p_source->getSampleSize() % 8 != 0)
			{
				TT_PANIC("Invalid sample size %d, must be a multiple of 8.", p_source->getSampleSize());
				return false;
			}
		}
		else
		{
			if (p_source->getSampleSize() != 8 && p_source->getSampleSize() != 16)
			{
				TT_PANIC("Invalid sample size %d, must be 8 or 16.", p_source->getSampleSize());
				return false;
			}
		}
		
		bufferLengthInMs = static_cast<u32>((p_source->getBufferSize() * 1000) / p_source->getFramerate());
		
		// set up a format
		WAVEFORMATEX format = {0};
		format.wFormatTag      = static_cast<WORD>((p_source->getChannelCount() > 2) ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM);
		format.nChannels       = static_cast<WORD>(p_source->getChannelCount());
		format.nSamplesPerSec  = static_cast<DWORD>(p_source->getFramerate());
		format.nAvgBytesPerSec = static_cast<DWORD>(p_source->getFramerate() * p_source->getChannelCount() * (p_source->getSampleSize() / 8));
		format.nBlockAlign     = static_cast<WORD>(p_source->getChannelCount() * (p_source->getSampleSize() / 8));
		format.wBitsPerSample  = static_cast<WORD>(p_source->getSampleSize());
		format.cbSize          = static_cast<WORD>(sizeof(format));
		
		HRESULT hr = p_engine->CreateSourceVoice(&voice,
		                                         &format,
		                                         0,
		                                         XAUDIO2_DEFAULT_FREQ_RATIO,
		                                         this,
		                                         0,
		                                         0);
		if (FAILED(hr))
		{
			TT_PANIC("Unable to create source voice: 0x%08X.", hr);
			return false;
		}
		
		bufferCount = BufferCount;
		buffers = new StreamBuffer[bufferCount];
		for (size_type i = 0; i < bufferCount; ++i)
		{
			buffers[i].buffer.pAudioData = new BYTE[p_source->getBufferSize() * (p_source->getSampleSize() / 8) * p_source->getChannelCount()];
			buffers[i].buffer.AudioBytes = static_cast<UINT32>(p_source->getBufferSize() * (p_source->getSampleSize() / 8) * p_source->getChannelCount());
			buffers[i].buffer.pContext = buffers + i;
			if (buffers[i].buffer.pAudioData == 0)
			{
				TT_PANIC("Failed to allocate audio buffer for stream.");
				free();
				return false;
			}
		}
		source = p_source;
		
		return true;
	}
	
	void free()
	{
		if (voice != 0)
		{
			voice->DestroyVoice();
			voice = 0;
		}
		source = 0;
		hwPlaying = false;
		playing   = false;
		paused    = false;
		bufferLengthInMs = 0;
		
		if (buffers != 0)
		{
			for (size_type i = 0; i < bufferCount; ++i)
			{
				delete[] buffers[i].buffer.pAudioData;
			}
			delete[] buffers;
			bufferCount = 0;
		}
	}
	
	bool play(bool p_updateAfterPlay)
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to start an uninitialized stream.");
			return false;
		}
		
		if (playing)
		{
			TT_PANIC("Attempt to start a stream that's already playing.");
			return false;
		}
		
		playing = true;
		paused  = false;
		return p_updateAfterPlay ? update() : true;
	}
	
	bool stop()
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to start an uninitialized stream.");
			return false;
		}
		
		if (playing == false)
		{
			TT_PANIC("Attempt to stop a stream that's already stopped.");
			return false;
		}
		
		if (paused == false)
		{
			// stop the voice
			HRESULT hr = voice->Stop(0, XAUDIO2_COMMIT_NOW);
			if (FAILED(hr))
			{
				TT_PANIC("Unable to stop stream: 0x%08X.", hr);
				return false;
			}
		}
		playing = false;
		paused  = false;
		
		HRESULT hr = voice->FlushSourceBuffers();
		if (FAILED(hr))
		{
			TT_PANIC("Unable to flush all buffers from stream: 0x%08X.", hr);
			return false;
		}
		
		return true;
	}
	
	bool pause()
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to pause uninitialized stream.");
			return false;
		}
		
		if (paused)
		{
			return true;
		}
		
		
		HRESULT hr = voice->Stop(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Unable to stop stream: 0x%08X.", hr);
			return false;
		}
		paused = true;
		return true;
	}
	
	bool resume()
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to resume uninitialized stream.");
			return false;
		}
		
		if (paused == false)
		{
			return true;
		}
		
		HRESULT hr = voice->Start(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Unable to start stream: 0x%08X.", hr);
			return false;
		}
		paused = false;
		return true;
	}
	
	bool update()
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to update uninitialized stream.");
			return false;
		}
		XAUDIO2_VOICE_STATE state = {0};
		voice->GetState(&state);
		
		// Check if playback passed the notification point
		if (notificationSample != 0 && state.SamplesPlayed >= notificationSample)
		{
			//TT_Printf("XAudio2SoundSystem::StreamData::update: Reached notification frame %llu! (play position %llu)\n",
			//          notificationSample, state.SamplesPlayed);
			notificationSample = 0;
			source->onStreamReachedNotificationFrame();
		}
		
		// update buffers
		while (playbackDone == false && state.BuffersQueued < BufferCount)
		{
			if (bufferData() == false)
			{
				return false;
			}
			voice->GetState(&state);
		}
		
		// make sure the stream is playing
		if (playing && hwPlaying == false)
		{
			HRESULT hr = voice->Start(0, XAUDIO2_COMMIT_NOW);
			if (FAILED(hr))
			{
				TT_PANIC("Unable to start stream: 0x%08X.", hr);
				return false;
			}
			hwPlaying = true;
		}
		return true;
	}
	
	bool bufferData()
	{
		if (source == 0)
		{
			return false;
		}
		
		for (size_type i = 0; i < bufferCount; ++i)
		{
			if (buffers[i].empty)
			{
				size_type bufferRelativeNotificationFrame = -1;
				size_type frames = source->fillBufferInterleaved(source->getBufferSize(), source->getChannelCount(), const_cast<BYTE*>(buffers[i].buffer.pAudioData), &bufferRelativeNotificationFrame);
				buffers[i].buffer.PlayLength = static_cast<UINT32>(frames);
				
				if (frames == 0)
				{
					// no frames, notify voice that the previous buffer is the last
					HRESULT hr = voice->Discontinuity();
					
					if (FAILED(hr))
					{
						TT_PANIC("Failed to notify voice of end of stream: 0x%08X.", hr);
						return false;
					}
					return true;
				}
				
				// If source wants a loop notification, translate the
				// buffer-relative position to an absolute play position
				if (bufferRelativeNotificationFrame >= 0)
				{
					notificationSample = totalSamplesBuffered + static_cast<UINT64>(bufferRelativeNotificationFrame);
				}
				
				if (frames != source->getBufferSize())
				{
					buffers[i].buffer.Flags = XAUDIO2_END_OF_STREAM;
					playbackDone = true;
				}
				else
				{
					buffers[i].buffer.Flags = 0;
				}
				
				HRESULT hr = voice->SubmitSourceBuffer(&buffers[i].buffer, 0);
				if (FAILED(hr))
				{
					TT_PANIC("Failed to submit buffer to stream: 0x%08X.", hr);
					return false;
				}
				buffers[i].empty = false;
				
				totalSamplesBuffered += static_cast<UINT64>(frames);
				break;
			}
		}
		return true;
	}
	
	bool setVolumeRatio(real p_ratio)
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to set volume of uninitialized stream.");
			return false;
		}
		
		float volume = static_cast<float>(p_ratio);
		if (volume < -XAUDIO2_MAX_VOLUME_LEVEL || volume > XAUDIO2_MAX_VOLUME_LEVEL)
		{
			TT_PANIC("Invalid volume %f, should be between %f and %f.",
			         volume, -XAUDIO2_MAX_VOLUME_LEVEL, XAUDIO2_MAX_VOLUME_LEVEL);
			return false;
		}
		
		HRESULT hr = voice->SetVolume(volume, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
		{
			TT_PANIC("Failed to set stream volume: 0x%08X.", hr);
			return false;
		}
		return true;
	}
	
	real getVolumeRatio()
	{
		if (voice == 0)
		{
			TT_PANIC("Attempt to get volume of uninitialized stream.");
			return real();
		}
		
		float volume = 0.0f;
		voice->GetVolume(&volume);
		
		return real(volume);
	}
	
	// IXAudio2VoiceCallback functions
	
	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32)
	{
	}
	
	STDMETHOD_(void, OnVoiceProcessingPassEnd)()
	{
	}
	
	STDMETHOD_(void, OnStreamEnd)()
	{
		hwPlaying = false;
		if (playbackDone)
		{
			playing = false;
		}
	}
	
	STDMETHOD_(void, OnBufferStart)(void*)
	{
	}
	
	STDMETHOD_(void, OnBufferEnd)(void* p_context)
	{
		// mark this buffer as unused
		StreamBuffer* buffer = reinterpret_cast<StreamBuffer*>(p_context);
		buffer->empty = true;
	}
	
	STDMETHOD_(void, OnLoopEnd)(void*)
	{
	}
	
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT)
	{
	}
};


//--------------------------------------------------------------------------------------------------
// Public member functions

SoundSystemPtr XAudio2SoundSystem::instantiate(identifier p_identifier, bool p_updateStreamsInThread)
{
	// create a sound system
	XAudio2SoundSystem* xa2 = new XAudio2SoundSystem(p_identifier, p_updateStreamsInThread);
	if (xa2 == 0)
	{
		TT_PANIC("Failed to instantiate soundsystem.");
		return SoundSystemPtr();
	}
	
	if (xa2->init() == false)
	{
		TT_PANIC("Failed to initialize XAudio2.");
		delete xa2;
		return SoundSystemPtr();
	}
	
	SoundSystemPtr sys(xa2);
	
	if (snd::registerSoundSystem(sys.get(), p_identifier) == false)
	{
		TT_PANIC("Failed to register soundsytem.");
		return SoundSystemPtr();
	}
	
	return sys;
}


XAudio2SoundSystem::~XAudio2SoundSystem()
{
	// clean up
	uninit();
	
	app::ComHelper::uninitCom();
}


// Master volume functions

real XAudio2SoundSystem::getMasterVolume()
{
	float ratio = 0.0f;
	TT_NULL_ASSERT(m_device);
	m_device->GetVolume(&ratio);
	return ratio;
}


void XAudio2SoundSystem::setMasterVolume(real p_volume)
{
	TT_NULL_ASSERT(m_device);
	HRESULT hr = m_device->SetVolume(static_cast<float>(p_volume), XAUDIO2_COMMIT_NOW);
	TT_ASSERTMSG(SUCCEEDED(hr), "Failed to set master volume. Error code: 0x%08X.", hr);
}


// Master suspend functions

bool XAudio2SoundSystem::suspend()
{
	return true;
}


bool XAudio2SoundSystem::resume()
{
	return true;
}


// Voice creation functions

bool XAudio2SoundSystem::openVoice(const VoicePtr& p_voice, size_type p_priority)
{
	(void)p_priority;
	TT_NULL_ASSERT(p_voice);
	if (p_voice->getData() != 0)
	{
		TT_PANIC("Voice already open.");
		return false;
	}
	
	VoiceData* data = new VoiceData;
	if (data->allocate() == false)
	{
		delete data;
		return false;
	}
	
	p_voice->setData(data);
	
	return true;
}


bool XAudio2SoundSystem::closeVoice(Voice* p_voice)
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

bool XAudio2SoundSystem::playVoice(const VoicePtr& p_voice, bool p_loop)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->play(p_loop) : false;
}


bool XAudio2SoundSystem::stopVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->stop() : false;
}


bool XAudio2SoundSystem::pauseVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->pause() : false;
}


bool XAudio2SoundSystem::resumeVoice(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->resume() : false;
}


// Voice Status functions

bool XAudio2SoundSystem::isVoicePlaying(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->isPlaying() : false;
}


bool XAudio2SoundSystem::isVoicePaused(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->isPaused() : false;
}


// Voice Parameter functions

BufferPtr XAudio2SoundSystem::getVoiceBuffer(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->buffer : BufferPtr();
}


bool XAudio2SoundSystem::setVoiceBuffer(const VoicePtr& p_voice, const BufferPtr& p_buffer)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setBuffer(p_buffer, m_engine) : false;
}


real XAudio2SoundSystem::getVoicePlaybackRatio(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->getPlaybackRatio() : 0.0f;
}


bool XAudio2SoundSystem::setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setPlaybackRatio(p_playbackRatio) : false;
}


real XAudio2SoundSystem::getVoiceVolumeRatio(const VoicePtr& p_voice)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->getVolumeRatio() : 0.0f;
}


bool XAudio2SoundSystem::setVoiceVolumeRatio(const VoicePtr& p_voice, real p_volumeRatio)
{
	VoiceData* data = getVoiceData(p_voice);
	return (data != 0) ? data->setVolumeRatio(p_volumeRatio) : false;
}


// Buffer creation functions

bool XAudio2SoundSystem::openBuffer(const BufferPtr& p_buffer)
{
	TT_NULL_ASSERT(p_buffer);
	if (p_buffer->getData() != 0)
	{
		TT_PANIC("Buffer already open.");
		return false;
	}
	
	BufferData* data = new BufferData;
	p_buffer->setData(data);
	
	return true;
}


bool XAudio2SoundSystem::closeBuffer(Buffer* p_buffer)
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


bool XAudio2SoundSystem::setBufferData(const BufferPtr& p_buffer,
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


size_type XAudio2SoundSystem::getBufferLength(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getFrameCount() : 0;
}


size_type XAudio2SoundSystem::getBufferChannelCount(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getChannelCount() : 0;
}


size_type XAudio2SoundSystem::getBufferSampleSize(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getSampleSize() : 0;
}


size_type XAudio2SoundSystem::getBufferSampleRate(const BufferPtr& p_buffer)
{
	BufferData* data = getBufferData(p_buffer);
	return (data != 0) ? data->getSampleRate() : 0;
}


// Stream Creation functions

bool XAudio2SoundSystem::openStream(const StreamPtr& p_stream)
{
	TT_NULL_ASSERT(p_stream);
	if (p_stream->getData() != 0)
	{
		TT_PANIC("Stream already open.");
		return false;
	}
	
	StreamSource* source = p_stream->getSource();
	if (source == 0)
	{
		TT_PANIC("Stream has no source.");
		return false;
	}
	
	StreamData* data = new StreamData;
	if (data->allocate(m_engine, source) == false)
	{
		TT_PANIC("Stream allocation failed.");
		delete data;
		return false;
	}
	
	data->bufferData();
	
	p_stream->setData(data);
	
	if (m_updateStreamsInThread)
	{
		thread::CriticalSection criticalSection(&m_decodeMutex);
		m_streamsToAdd.push_back(p_stream.get());
	}
	
	return true;
}


bool XAudio2SoundSystem::closeStream(Stream* p_stream)
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
	if (data->playing)
	{
		TT_PANIC("Stream still playing, stop first!");
	}
	
	delete data;
	p_stream->setData(0);
	
	return true;
}


// Stream Playback functions

bool XAudio2SoundSystem::playStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	if (data == 0)
	{
		return false;
	}
	
	if (data->playing)
	{
		TT_PANIC("Stream already playing.");
		return false;
	}
	
	const bool updateAfterPlay = (m_updateStreamsInThread == false);
	if (data->play(updateAfterPlay) == false)
	{
		TT_PANIC("Failed to start stream.");
		return false;
	}
	
	return true;
}


bool XAudio2SoundSystem::stopStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	if (data == 0)
	{
		return false;
	}
	
	if (data->playing == false)
	{
		TT_WARN("Stopping stream that isn't playing.");
		return true;
	}
	
	return data->stop();
}


bool XAudio2SoundSystem::pauseStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	if (data == 0)
	{
		return false;
	}
	
	if (data->playing == false)
	{
		TT_PANIC("Cannot pause non-playing stream.");
		return false;
	}
	
	if (data->paused)
	{
		TT_WARN("Stream already paused.");
		return true;
	}
	
	if (data->pause() == false)
	{
		TT_PANIC("Failed to pause stream.");
		return false;
	}
	
	return true;
}


bool XAudio2SoundSystem::resumeStream(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	if (data == 0)
	{
		return false;
	}
	
	if (data->playing == false)
	{
		TT_PANIC("Cannot resume non-playing stream.");
		return false;
	}
	
	if (data->paused == false)
	{
		TT_WARN("Stream already resumed.");
		return true;
	}
	
	if (data->resume() == false)
	{
		TT_PANIC("Failed to resume stream.");
		return false;
	}
	
	return true;
}


bool XAudio2SoundSystem::updateStream(const StreamPtr& p_stream)
{
	if (m_updateStreamsInThread)
	{
		return true;
	}
	
	StreamData* data = getStreamData(p_stream);
	if (data == 0)
	{
		return false;
	}
	
	if (data->playing == false)
	{
		//TT_PANIC("Updating non playing stream.");
		return false;
	}
	
	if (data->update() == false)
	{
		TT_PANIC("Failed to update stream.");
		return false;
	}
	
	return true;
}


bool XAudio2SoundSystem::isStreamPlaying(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->playing : false;
}


bool XAudio2SoundSystem::isStreamPaused(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->paused : false;
}


// Stream Parameter functions

real XAudio2SoundSystem::getStreamVolumeRatio(const StreamPtr& p_stream)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->getVolumeRatio() : 0.0f;
}


bool XAudio2SoundSystem::setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio)
{
	StreamData* data = getStreamData(p_stream);
	return (data != 0) ? data->setVolumeRatio(p_volumeRatio) : false;
}


real XAudio2SoundSystem::getStreamVolume(const StreamPtr& p_stream)
{
	return audio::helpers::ratioTodB(getStreamVolumeRatio(p_stream));
}


bool XAudio2SoundSystem::setStreamVolume(const StreamPtr& p_stream, real p_volume)
{
	return setStreamVolumeRatio(p_stream, audio::helpers::dBToRatio(p_volume));
}


//--------------------------------------------------------------------------------------------------
// Private member functions

XAudio2SoundSystem::XAudio2SoundSystem(identifier p_identifier, bool p_updateStreamsInThread)
:
SoundSystem(p_identifier),
m_engine(0),
m_device(0),
m_updateStreamsInThread(p_updateStreamsInThread),
m_decodeThread(),
m_decodeThreadShouldExit(false),
m_streamsToAdd(),
m_activeStreams(),
m_streamToRemove(0),
m_decodeMutex(),
m_decodeSemaphore()
{
	app::ComHelper::initCom();
}


bool XAudio2SoundSystem::init()
{
	// Create an XAudio2 Interface
	HRESULT hr = XAudio2Create(&m_engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		TT_PANIC("Failed to create XAudio2 device: 0x%08X.", hr);
		return false;
	}
	
	if (FAILED(hr = m_engine->CreateMasteringVoice(&m_device,
	                                               XAUDIO2_DEFAULT_CHANNELS,
	                                               XAUDIO2_DEFAULT_SAMPLERATE,
	                                               0,
	                                               0,
	                                               NULL)))
	{
		TT_PANIC("Failed to create Mastering voice: 0x%08X.", hr);
		uninit();
		return false;
	}
	
	
	if (m_updateStreamsInThread)
	{
		m_decodeSemaphore.create(0);
		if (m_decodeSemaphore.isValid() == false)
		{
			TT_PANIC("Failed to create a semaphore for stream decoding thread communication.");
			uninit();
			return false;
		}
		
		m_decodeThreadShouldExit = false;
		m_decodeThread = thread::create(staticDecodeStreamsThread, this, true, 0);
		if (m_decodeThread == 0)
		{
			TT_PANIC("Failed to create the stream decoding thread.");
			uninit();
			return false;
		}
		
		thread::setName(m_decodeThread, "Audio Decoder Thread");
		thread::resume(m_decodeThread);
	}
	
	return true;
}


bool XAudio2SoundSystem::uninit()
{
	if (m_updateStreamsInThread)
	{
		if (m_decodeThread != 0)
		{
			// Tell decode thread to exit and wait for the thread to die
			{
				thread::CriticalSection criticalSection(&m_decodeMutex);
				m_decodeThreadShouldExit = true;
			}
			thread::wait(m_decodeThread);
			m_decodeThread.reset();
		}
		
		m_decodeSemaphore.destroy();
	}
	
	if (m_device != 0)
	{
		m_device->DestroyVoice();
		m_device = 0;
	}
	if (m_engine != 0)
	{
		m_engine->Release();
		m_engine = 0;
	}
	
	return true;
}


XAudio2SoundSystem::VoiceData* XAudio2SoundSystem::getVoiceData(const VoicePtr& p_voice)
{
	return checkVoiceOpen(p_voice) ? reinterpret_cast<VoiceData*>(p_voice->getData()) : 0;
}


XAudio2SoundSystem::BufferData* XAudio2SoundSystem::getBufferData(const BufferPtr& p_buffer)
{
	return checkBufferOpen(p_buffer) ? reinterpret_cast<BufferData*>(p_buffer->getData()) : 0;
}


XAudio2SoundSystem::StreamData* XAudio2SoundSystem::getStreamData(const StreamPtr& p_stream)
{
	return checkStreamOpen(p_stream) ? reinterpret_cast<StreamData*>(p_stream->getData()) : 0;
}


int XAudio2SoundSystem::staticDecodeStreamsThread(void* p_soundSystem)
{
	TT_NULL_ASSERT(p_soundSystem);
	static_cast<XAudio2SoundSystem*>(p_soundSystem)->decodeStreams();
	return 0;
}


void XAudio2SoundSystem::decodeStreams()
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
		//OSWaitEvent(&m_audioSuspendEvent);  // FIXME: Does Windows need something like this too?
		
		// Check for new streams
		{
			thread::CriticalSection criticalSection(&m_decodeMutex);
			
			if (m_decodeThreadShouldExit)
			{
				TT_Printf("XAudio2SoundSystem::decodeStreams: Someone wants me to leave... bye bye!\n");
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
			TT_NULL_ASSERT(*it);
			
			StreamData* stream = reinterpret_cast<StreamData*>((*it)->getData());
			TT_NULL_ASSERT(stream);
			
			if (stream->playing)
			{
				stream->update();
			}
			
			if (stream->bufferLengthInMs > 0 &&
			    stream->bufferLengthInMs < shortestBufferInMs)
			{
				shortestBufferInMs = stream->bufferLengthInMs;
			}
		}
		
		const u32 updateMs = static_cast<u32>(tm->getMilliSeconds() - updateStart);
#if SHOW_STREAM_TIMINGS
		if (updateMs > 25 && m_activeStreams.empty() == false)
		{
			TT_Printf("XAudio2SoundSystem::decodeStreams: Updating %u streams took %u ms.\n",
			          m_activeStreams.size(), updateMs);
		}
#endif
		
		// Check for stream to remove
		{
			thread::CriticalSection criticalSection(&m_decodeMutex);
			
			if (m_streamToRemove != 0)
			{
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
