#include <algorithm>
#include <tt/audio/codec/wav/WavFormatDecoderPcm.h>
#include <tt/fs/File.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {


WavFormatDecoderPcm::WavFormatDecoderPcm(WavFormat& p_format, const fs::FilePtr& p_file)
:
WavFormatDecoder(p_format, p_file),
m_sampleCount(0),
m_frameCount(0),
m_samplesPerBlock(0),
m_framesPerBlock(0),
m_position(0)
{
	m_samplesPerBlock = m_format.blockSize / (m_format.sampleSize / 8);
	m_framesPerBlock  = m_samplesPerBlock / m_format.channelCount;
}


WavFormatDecoderPcm::~WavFormatDecoderPcm()
{
}


void WavFormatDecoderPcm::parseFact(size_type p_chunkSize)
{
	// no fact chunk for PCM
	(void)p_chunkSize;
	return;
}


void WavFormatDecoderPcm::parseExFormat(size_type p_chunkSize)
{
	// no ex format for PCM
	(void)p_chunkSize;
	return;
}


void WavFormatDecoderPcm::parseData(size_type p_chunkSize)
{
	m_sampleCount = p_chunkSize / (m_format.sampleSize / 8);
	m_frameCount = m_sampleCount / m_format.channelCount;
	m_rewindPosition = m_file->getPosition();
	return;
}


size_type WavFormatDecoderPcm::getSampleCount() const
{
	return m_sampleCount;
}


size_type WavFormatDecoderPcm::getFrameCount() const
{
	return m_frameCount;
}


size_type WavFormatDecoderPcm::getSamplesPerBlock() const
{
	return m_samplesPerBlock;
}


size_type WavFormatDecoderPcm::getFramesPerBlock() const
{
	return m_framesPerBlock;
}


SampleType WavFormatDecoderPcm::getSampleType() const
{
	switch (m_format.sampleSize)
	{
	case 8:
		return SampleType_Unsigned8;
	case 16:
		return SampleType_Signed16;
	default:
		return SampleType_Unknown;
	}
}


size_type WavFormatDecoderPcm::getBlockBufferSize() const
{
	switch (m_format.sampleSize)
	{
	case 8:
		return m_framesPerBlock * m_format.channelCount;
	case 16:
		return static_cast<size_type>(m_framesPerBlock * m_format.channelCount * sizeof(s16));
	default:
		return 0;
	}
}


size_type WavFormatDecoderPcm::decodeBlocks(void** p_buffer, size_type p_blocks)
{
	// calculate how many frames to read
	size_type frames = std::min(p_blocks * m_framesPerBlock, m_frameCount - m_position);
	
	// calculate how many bytes to read
	size_type size = frames * m_format.channelCount * (m_format.sampleSize / 8);
	if (size == 0)
	{
		return 0;
	}
	
	void* buffer = ::operator new(static_cast<size_t>(size));
	if (buffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", size);
		return 0;
	}
	fs::size_type toRead = static_cast<fs::size_type>(size);
	fs::size_type read = m_file->read(buffer, toRead);
	size = static_cast<size_type>(read);
	if (size == 0)
	{
		::operator delete(buffer);
		return 0;
	}
	
	size_type framesRead = size / (m_format.channelCount * (m_format.sampleSize / 8));
	
	m_position += framesRead;
	
	switch (getSampleType())
	{
	case SampleType_Unsigned8:
		{
			u8** destBuffer = reinterpret_cast<u8**>(p_buffer);
			u8* sourceBuffer = reinterpret_cast<u8*>(buffer);
			for (size_type frame = 0; frame < framesRead; ++frame)
			{
				for (size_type channel = 0; channel < m_format.channelCount; ++channel)
				{
					destBuffer[channel][frame] = *sourceBuffer;
					++sourceBuffer;
				}
			}
		}
		::operator delete(buffer);
		return framesRead;
		
	case SampleType_Signed16:
		{
			s16** destBuffer = reinterpret_cast<s16**>(p_buffer);
			s16* sourceBuffer = reinterpret_cast<s16*>(buffer);
			for (size_type frame = 0; frame < framesRead; ++frame)
			{
				for (size_type channel = 0; channel < m_format.channelCount; ++channel)
				{
					destBuffer[channel][frame] = *sourceBuffer;
					++sourceBuffer;
				}
			}
		}
		::operator delete(buffer);
		return framesRead;
		
	default:
		::operator delete(buffer);
		return 0;
	}
}


size_type WavFormatDecoderPcm::decodeBlocksInterleaved(void* p_buffer, size_type p_blocks)
{
	// calculate how many frames to read
	size_type frames = std::min(p_blocks * m_framesPerBlock, m_frameCount - m_position);
	
	// calculate how many bytes to read
	size_type size = frames * m_format.channelCount * (m_format.sampleSize / 8);
	if (size == 0)
	{
		return 0;
	}
	
	fs::size_type toRead = static_cast<fs::size_type>(size);
	fs::size_type read = m_file->read(p_buffer, toRead);
	size = static_cast<size_type>(read);
	if (size == 0)
	{
		return 0;
	}
	
	size_type framesRead = size / (m_format.channelCount * (m_format.sampleSize / 8));
	
	m_position += framesRead;
	
	return framesRead;
}


bool WavFormatDecoderPcm::rewind()
{
	if (m_file->seek(m_rewindPosition, fs::SeekPos_Set) == false)
	{
		return false;
	}
	m_position = 0;
	return true;
}

// namespace end
}
}
}
}
