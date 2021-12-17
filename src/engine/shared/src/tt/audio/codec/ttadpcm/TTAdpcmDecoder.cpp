#include <cstring>
#include <algorithm>
#include <tt/audio/codec/ttadpcm/TTAdpcmDecoder.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace codec {
namespace ttadpcm {


TTAdpcmDecoder::TTAdpcmDecoder(const std::string& p_filename, fs::identifier p_fs)
:
Decoder(),
m_file(fs::open(p_filename, fs::OpenMode_Read, p_fs)),
m_rewindPosition(0),
m_position(0),
m_sampleType(SampleType_Signed16),
m_channelCount(0),
m_sampleRate(0),
m_sampleSize(16),
m_frameCount(0),
m_decodeStates(),
m_leftOver(0),
m_hasLeftOver(false)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	// get file information
	
	char id[8];
	m_file->read(id, 8);
	if (std::memcmp(id, "TTADPCM", 8) != 0)
	{
		TT_PANIC("'%s' is not a valid ttadpcm file.", p_filename.c_str());
		return;
	}
	
	u32 frameCount = 0;
	fs::readInteger(m_file, &frameCount);
	
	u32 channelCount = 0;
	fs::readInteger(m_file, &channelCount);
	
	u32 sampleRate = 0;
	fs::readInteger(m_file, &sampleRate);
	
	m_frameCount   = static_cast<size_type>(frameCount);
	m_channelCount = static_cast<size_type>(channelCount);
	m_sampleRate   = static_cast<size_type>(sampleRate);
	
	m_decodeStates.resize(static_cast<DecodeStates::size_type>(m_channelCount));
	
	m_rewindPosition = m_file->getPosition();
	for (size_type i = 0; i < m_channelCount; ++i)
	{
		// read header per channel
		s16 predictor = 0;
		fs::readInteger(m_file, &predictor);
		s8 stepIndex = 0;
		fs::readInteger(m_file, &stepIndex);
		s8 pad = 0;
		fs::readInteger(m_file, &pad);
		
		m_decodeStates[static_cast<DecodeStates::size_type>(i)].predictor = static_cast<int>(predictor);
		m_decodeStates[static_cast<DecodeStates::size_type>(i)].stepIndex = static_cast<int>(stepIndex);
	}
}


TTAdpcmDecoder::~TTAdpcmDecoder()
{
}


size_type TTAdpcmDecoder::getChannelCount() const
{
	return m_channelCount;
}


size_type TTAdpcmDecoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type TTAdpcmDecoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type TTAdpcmDecoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType TTAdpcmDecoder::getSampleType() const
{
	return m_sampleType;
}


size_type TTAdpcmDecoder::decode(SampleType p_type,
                                 void**     p_buffer,
                                 size_type  p_channels,
                                 size_type  p_frames,
                                 size_type  p_offset)
{
	size_type todo = std::min(p_frames, m_frameCount - m_position);
	
	// calculate number of samples to read
	size_type size = (todo * m_channelCount);
	if (m_hasLeftOver)
	{
		// still one sample remaining in internal buffer
		--size;
	}
	
	if ((size & 1) != 0)
	{
		++size;
	}
	size /= 2;
	
	u8* data = new u8[size];
	if (data == 0)
	{
		TT_PANIC("Error allocating %d bytes.", size);
		return size_type();
	}
	u8* scratch = data;
	
	fs::size_type toRead = static_cast<fs::size_type>(size);
	fs::size_type read = m_file->read(data, toRead);
	
	if (read != toRead)
	{
		TT_PANIC("Error reading %d bytes (%d frames) from %s, got %d bytes.", toRead, todo, m_file->getPath(), read);
		delete[] data;
		return size_type();
	}
	
	m_position += todo;
	
	// decode to buffer
	for (size_type frame = 0; frame < todo; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			if (channel >= p_channels)
			{
				continue;
			}
			
			u8 sampleSource;
			if (m_hasLeftOver)
			{
				sampleSource = static_cast<u8>(m_leftOver >> 4);
				m_hasLeftOver = false;
			}
			else
			{
				m_leftOver = *scratch;
				++scratch;
				m_hasLeftOver = true;
				sampleSource = static_cast<u8>(m_leftOver & 0x0F);
			}
			
			s16 sample = ttadpcm::decode(m_decodeStates[static_cast<DecodeStates::size_type>(channel)], sampleSource);
			
			switch (p_type)
			{
			case SampleType_Signed8:
				reinterpret_cast<s8**>(p_buffer)[channel][frame + p_offset] = static_cast<s8>(sample / 256);
				break;
				
			case SampleType_Unsigned8:
				reinterpret_cast<u8**>(p_buffer)[channel][frame + p_offset] = static_cast<u8>((sample / 256) + 128);
				break;
				
			case SampleType_Signed16:
				reinterpret_cast<s16**>(p_buffer)[channel][frame + p_offset] = sample;
				break;
				
			case SampleType_Float:
				reinterpret_cast<float**>(p_buffer)[channel][frame + p_offset] = static_cast<float>(sample) / 32767.0f;
				break;
				
			default:
				break;
			}
		}
	}
	
	delete[] data;
	
	return todo;
}


size_type TTAdpcmDecoder::decodeInterleaved(SampleType p_type,
                                            void*      p_buffer,
                                            size_type  p_channels,
                                            size_type  p_frames,
                                            size_type  p_offset)
{
	size_type todo = std::min(p_frames, m_frameCount - m_position);
	
	// calculate number of samples to read
	size_type size = (todo * m_channelCount);
	if (m_hasLeftOver)
	{
		// still one sample remaining in internal buffer
		--size;
	}
	
	if ((size & 1) != 0)
	{
		++size;
	}
	size /= 2;
	
	u8* data = new u8[size];
	if (data == 0)
	{
		TT_PANIC("Error allocating %d bytes.", size);
		return size_type();
	}
	u8* scratch = data;
	
	fs::size_type toRead = static_cast<fs::size_type>(size);
	fs::size_type read = m_file->read(data, toRead);
	
	if (read != toRead)
	{
		TT_PANIC("Error reading %d bytes (%d frames) from %s, got %d bytes.", toRead, todo, m_file->getPath(), read);
		delete[] data;
		return size_type();
	}
	
	m_position += todo;
	
	// decode to buffer
	for (size_type frame = 0; frame < todo; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			if (channel >= p_channels)
			{
				continue;
			}
			
			u8 sampleSource;
			if (m_hasLeftOver)
			{
				sampleSource = static_cast<u8>(m_leftOver >> 4);
				m_hasLeftOver = false;
			}
			else
			{
				m_leftOver = *scratch;
				++scratch;
				m_hasLeftOver = true;
				sampleSource = static_cast<u8>(m_leftOver & 0x0F);
			}
			
			s16 sample = ttadpcm::decode(m_decodeStates[static_cast<DecodeStates::size_type>(channel)], sampleSource);
			
			size_type destIndex = ((frame + p_offset) * p_channels) + channel;
			
			switch (p_type)
			{
			case SampleType_Signed8:
				reinterpret_cast<s8*>(p_buffer)[destIndex] = static_cast<s8>(sample / 256);
				break;
				
			case SampleType_Unsigned8:
				reinterpret_cast<u8*>(p_buffer)[destIndex] = static_cast<u8>((sample / 256) + 128);
				break;
				
			case SampleType_Signed16:
				reinterpret_cast<s16*>(p_buffer)[destIndex] = sample;
				break;
				
			case SampleType_Float:
				reinterpret_cast<float*>(p_buffer)[destIndex] = static_cast<float>(sample) / 32767.0f;
				break;
				
			default:
				break;
			}
		}
	}
	
	delete[] data;
	
	return todo;
}


bool TTAdpcmDecoder::rewind()
{
	if (m_file->seek(m_rewindPosition, fs::SeekPos_Set) == false)
	{
		return false;
	}
	
	for (size_type i = 0; i < m_channelCount; ++i)
	{
		// read header per channel
		s16 predictor = 0;
		fs::readInteger(m_file, &predictor);
		s8 stepIndex = 0;
		fs::readInteger(m_file, &stepIndex);
		s8 pad = 0;
		fs::readInteger(m_file, &pad);
		
		m_decodeStates[static_cast<DecodeStates::size_type>(i)].predictor = static_cast<int>(predictor);
		m_decodeStates[static_cast<DecodeStates::size_type>(i)].stepIndex = static_cast<int>(stepIndex);
	}
	return true;
}

// namespace end
}
}
}
}
