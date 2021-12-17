#include <tt/audio/xact/Wave.h>
#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/WaveBank.h>
#include <tt/audio/xact/WaveInstance.h>
#include <tt/audio/codec/ttadpcm/TTAdpcmDecoder.h>
#include <tt/audio/codec/wav/WavDecoder.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/Buffer.h>
#include <tt/snd/snd.h>
#include <tt/xml/XmlNode.h>


//#define WAVE_DEBUG
#ifdef WAVE_DEBUG
#define Wave_Printf TT_Printf
#else
#define Wave_Printf(...)
#endif

#define WAVE_WARN
#ifdef WAVE_WARN
#define Wave_Warn TT_Printf
#else
#define Wave_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

Wave::Wave()
:
m_fileName(),
m_silent(false),
m_buffer(),
m_duration(0.0f),
m_waveIndex(-1),
m_waveBankIndex(-1)
{
}



int Wave::getSampleRate() const
{
	if (m_buffer == 0)
	{
		return 0;
	}
	return static_cast<int>(m_buffer->getSampleRate());
}


Wave* Wave::createWave(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	std::string filename          = bu::get<std::string     >(p_context);
	codec::size_type frameCount   = bu::get<codec::size_type>(p_context);
	codec::size_type sampleRate   = bu::get<codec::size_type>(p_context);
	bool isSilent                 = bu::get<bool            >(p_context);
	
	if (p_context->statusCode != 0)
	{
		return 0;
	}
	
	Wave* wave = new Wave();
	wave->m_duration = frameCount / static_cast<real>(sampleRate);
	wave->m_fileName = filename;
	wave->m_silent   = isSilent;
	
	if (isSilent)
	{
		// Nothing more to load for a silent wave at this point; early out
		return wave;
	}
	
	// Non silent wave; load remaining data
	codec::size_type channelCount = bu::get<codec::size_type>(p_context);
	codec::size_type sampleSize   = bu::get<codec::size_type>(p_context);
	u32 sizeInBytes               = bu::get<u32             >(p_context);
	u8* data = new u8[sizeInBytes];
	bu::get(data, sizeInBytes, p_context);
	
	if (p_context->statusCode != 0)
	{
		delete wave;
		delete [] data;
		return 0;
	}
	
	wave->m_buffer = snd::openBuffer(AudioTT::getSoundSystem());
	if (wave->m_buffer == 0)
	{
		TT_PANIC("Unable to create buffer with sound system %d.", AudioTT::getSoundSystem());
		delete wave;
		delete [] data;
		return 0;
	}
	
	// Set buffer data and transfer ownership of data[] to it
	if (wave->m_buffer->setBufferData(
		data, frameCount, channelCount, sampleSize, sampleRate, true) == false)
	{
		TT_PANIC("Failed to set buffer data.");
		delete [] data;
		wave->m_buffer.reset();
		delete wave;
		return 0;
	}
	
	return wave;
}


WaveInstance* Wave::instantiate()
{
	WaveInstance* instance = new WaveInstance(this);
	return instance;
}


s32 Wave::getMemorySize() const
{
	if (m_buffer == 0)
	{
		return 0;
	}
	
	return (m_buffer->getSampleSize() / 8) * m_buffer->getLength() * m_buffer->getChannelCount();
}

// namespace end
}
}
}
