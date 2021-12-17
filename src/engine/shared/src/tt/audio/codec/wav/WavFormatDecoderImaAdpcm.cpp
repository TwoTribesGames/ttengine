#include <tt/audio/codec/wav/WavFormatDecoderImaAdpcm.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {


WavFormatDecoderImaAdpcm::WavFormatDecoderImaAdpcm(WavFormat& p_format, const fs::FilePtr& p_file)
:
WavFormatDecoder(p_format, p_file),
m_sampleCount(0),
m_frameCount(0),
m_samplesPerBlock(0),
m_framesPerBlock(0),
m_blockCount(0),
m_position(0),
m_decodeStates()
{
}


WavFormatDecoderImaAdpcm::~WavFormatDecoderImaAdpcm()
{
}


void WavFormatDecoderImaAdpcm::parseFact(size_type p_chunkSize)
{
	if (p_chunkSize < 4)
	{
		return;
	}
	
	u32 sampleCount = 0;
	tt::fs::readInteger(m_file, &sampleCount);
	m_sampleCount = static_cast<size_type>(sampleCount);
	m_frameCount = m_sampleCount / m_format.channelCount;
	m_blockCount = (m_sampleCount + (m_samplesPerBlock - 1)) / m_samplesPerBlock;
}


void WavFormatDecoderImaAdpcm::parseExFormat(size_type p_chunkSize)
{
	if (p_chunkSize < 2)
	{
		return;
	}
	
	s16 samplesPerBlock = 0;
	tt::fs::readInteger(m_file, &samplesPerBlock);
	m_samplesPerBlock = static_cast<size_type>(samplesPerBlock);
	m_framesPerBlock = samplesPerBlock / m_format.channelCount;
	
	m_decodeStates.resize(static_cast<DecodeStates::size_type>(m_format.channelCount));
}


void WavFormatDecoderImaAdpcm::parseData(size_type p_chunkSize)
{
	(void)p_chunkSize;
	m_rewindPosition = m_file->getPosition();
	return;
}


size_type WavFormatDecoderImaAdpcm::getSampleCount() const
{
	return m_sampleCount;
}


size_type WavFormatDecoderImaAdpcm::getFrameCount() const
{
	return m_frameCount;
}


size_type WavFormatDecoderImaAdpcm::getSamplesPerBlock() const
{
	return m_samplesPerBlock;
}


size_type WavFormatDecoderImaAdpcm::getFramesPerBlock() const
{
	return m_framesPerBlock;
}


SampleType WavFormatDecoderImaAdpcm::getSampleType() const
{
	return SampleType_Signed16;
}


size_type WavFormatDecoderImaAdpcm::getBlockBufferSize() const
{
	return static_cast<size_type>(sizeof(s16) * m_framesPerBlock * m_format.channelCount);
}


size_type WavFormatDecoderImaAdpcm::decodeBlocks(void** p_buffer, size_type p_blocks)
{
	if (p_blocks == 0)
	{
		return 0;
	}
	
	// allocate buffer size of block
	void* buffer = ::operator new(static_cast<size_t>(m_format.blockSize));
	if (buffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_format.blockSize);
		return 0;
	}
	fs::size_type blockSize = static_cast<fs::size_type>(m_format.blockSize);
	size_type framesRead = 0;
	
	s16** destBuffer = new s16*[m_format.channelCount];
	for (size_type channel = 0; channel < m_format.channelCount; ++channel)
	{
		destBuffer[channel] = reinterpret_cast<s16*>(p_buffer[channel]);
	}
	
	for (size_type i = 0; i < p_blocks; ++i)
	{
		// read block
		fs::size_type read = m_file->read(buffer, blockSize);
		size_type framesInBlock = 0;
		if (m_position < (m_blockCount - 1))
		{
			// not last block
			if (read != blockSize)
			{
				TT_PANIC("Unable to read block %d completely, expected %d bytes, got %d\n",
				         m_position, blockSize, read);
				::operator delete(buffer);
				delete[] destBuffer;
				return framesRead;
			}
			framesInBlock = m_framesPerBlock;
		}
		else
		{
			framesInBlock = m_frameCount % m_framesPerBlock;
		}
		++m_position;
		
		const u8* scratch = reinterpret_cast<u8*>(buffer);
		size_t remaining = static_cast<size_t>(read);
		
		// read block header
		for (size_type channel = 0; channel < m_format.channelCount; ++channel)
		{
			s16 predictor = code::bufferutils::get<s16>(scratch, remaining);
			s8  stepIndex = code::bufferutils::get<s8>(scratch, remaining);
			/*s8  pad       =*/ code::bufferutils::get<s8>(scratch, remaining);
			
			DecodeStates::size_type index = static_cast<DecodeStates::size_type>(channel);
			m_decodeStates[index].predictor = static_cast<int>(predictor);
			m_decodeStates[index].stepIndex = static_cast<int>(stepIndex);
			*destBuffer[channel] = predictor;
			++(destBuffer[channel]);
		}
		
		--framesInBlock; // we already read one frame in the header
		++framesRead;
		
		// ima adpcm is stored interleaved with 8 samples as smallest unit
		// i.e. 8 left / 8 right / 8 left / 8 right / etc
		// calculate amount of 
		for (size_type frame = 0; frame < framesInBlock;)
		{
			size_type framesRemaining = std::min(size_type(8), framesInBlock - frame);
			framesRead += framesRemaining;
			for (size_type channel = 0; channel < m_format.channelCount; ++channel)
			{
				size_type frames = framesRemaining;
				for (size_type index = 0; index < 4; ++index)
				{
					u8 data = *scratch;
					++scratch;
					if (frames > 0)
					{
						DecodeStates::size_type idx = static_cast<DecodeStates::size_type>(channel);
						*destBuffer[channel] = ttadpcm::decode(m_decodeStates[idx], static_cast<u8>(data & 0x0F));
						++(destBuffer[channel]);
						--frames;
						if (frames > 0)
						{
							*destBuffer[channel] = ttadpcm::decode(m_decodeStates[idx], static_cast<u8>(data >> 4));
							++(destBuffer[channel]);
							--frames;
						}
					}
				}
			}
			frame += 8;
		}
	}
	delete[] destBuffer;
	::operator delete(buffer);
	
	return framesRead;
}


size_type WavFormatDecoderImaAdpcm::decodeBlocksInterleaved(void* p_buffer, size_type p_blocks)
{
	if (p_blocks == 0)
	{
		return 0;
	}
	
	// allocate buffer size of block
	void* buffer = ::operator new(static_cast<size_t>(m_format.blockSize));
	if (buffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_format.blockSize);
		return 0;
	}
	fs::size_type blockSize = static_cast<fs::size_type>(m_format.blockSize);
	size_type framesRead = 0;
	
	s16* destBuffer = reinterpret_cast<s16*>(p_buffer);
	
	for (size_type i = 0; i < p_blocks; ++i)
	{
		// read block
		fs::size_type read = m_file->read(buffer, blockSize);
		size_type framesInBlock = 0;
		if (m_position < (m_blockCount - 1))
		{
			// not last block
			if (read != blockSize)
			{
				TT_PANIC("Unable to read block %d completely, expected %d bytes, got %d\n",
				         m_position, blockSize, read);
				::operator delete(buffer);
				return framesRead;
			}
			framesInBlock = m_framesPerBlock;
		}
		else
		{
			framesInBlock = m_frameCount % m_framesPerBlock;
		}
		++m_position;
		
		const u8* scratch = reinterpret_cast<u8*>(buffer);
		size_t remaining = static_cast<size_t>(read);
		
		// read block header
		for (size_type channel = 0; channel < m_format.channelCount; ++channel)
		{
			s16 predictor = code::bufferutils::get<s16>(scratch, remaining);
			s8  stepIndex = code::bufferutils::get<s8>(scratch, remaining);
			/*s8  pad     =*/ code::bufferutils::get<s8>(scratch, remaining);
			
			DecodeStates::size_type index = static_cast<DecodeStates::size_type>(channel);
			m_decodeStates[index].predictor = static_cast<int>(predictor);
			m_decodeStates[index].stepIndex = static_cast<int>(stepIndex);
			*destBuffer = predictor;
			++destBuffer;
		}
		
		--framesInBlock; // we already read one frame in the header
		++framesRead;
		
		// ima adpcm is stored interleaved with 8 samples as smallest unit
		// i.e. 8 left / 8 right / 8 left / 8 right / etc
		for (size_type frame = 0; frame < framesInBlock;)
		{
			size_type framesRemaining = std::min(size_type(8), framesInBlock - frame);
			framesRead += framesRemaining;
			for (size_type channel = 0; channel < m_format.channelCount; ++channel)
			{
				size_type frames = framesRemaining;
				for (size_type index = 0; index < 4; ++index)
				{
					u8 data = *scratch;
					++scratch;
					if (frames > 0)
					{
						DecodeStates::size_type idx = static_cast<DecodeStates::size_type>(channel);
						*destBuffer = ttadpcm::decode(m_decodeStates[idx], static_cast<u8>(data & 0x0F));
						++destBuffer;
						--frames;
						if (frames > 0)
						{
							*destBuffer = ttadpcm::decode(m_decodeStates[idx], static_cast<u8>(data >> 4));
							++destBuffer;
							--frames;
						}
					}
				}
			}
			frame += 8;
		}
	}
	::operator delete(buffer);
	
	return framesRead;
}


bool WavFormatDecoderImaAdpcm::rewind()
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
