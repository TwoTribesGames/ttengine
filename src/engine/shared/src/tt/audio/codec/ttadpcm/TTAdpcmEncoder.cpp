#include <tt/audio/codec/ttadpcm/TTAdpcmEncoder.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace codec {
namespace ttadpcm {

TTAdpcmEncoder::TTAdpcmEncoder(size_type          p_channelCount,
                               size_type          p_sampleRate,
                               const std::string& p_filename,
                               fs::identifier     p_fs)
:
Encoder(),
m_file(fs::open(p_filename, fs::OpenMode_Write, p_fs)),
m_sampleType(SampleType_Signed16),
m_channelCount(p_channelCount),
m_sampleRate(p_sampleRate),
m_sampleSize(16),
m_frameCount(0),
m_encodeStates(static_cast<EncodeStates::size_type>(p_channelCount)),
m_leftOver(0),
m_hasLeftOver(false)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	// write file information
	m_file->write("TTADPCM", 8);
	
	// dummy size value
	u32 frames = 0;
	fs::writeInteger(m_file, frames);
	
	u32 channels = static_cast<u32>(m_channelCount);
	fs::writeInteger(m_file, channels);
	
	u32 sampleRate = static_cast<u32>(m_sampleRate);
	fs::writeInteger(m_file, sampleRate);
	
	// data follows
}


TTAdpcmEncoder::~TTAdpcmEncoder()
{
	if (m_file == 0)
	{
		// file not open, error in constructor
		return;
	}
	
	if (m_hasLeftOver)
	{
		tt::fs::writeInteger(m_file, m_leftOver);
	}
	
	m_file->seek(8, fs::SeekPos_Set); // 8 bytes from begin, frame count
	u32 frameCount = static_cast<u32>(m_frameCount);
	fs::writeInteger(m_file, frameCount);
	
	// all done
}


size_type TTAdpcmEncoder::getChannelCount() const
{
	return m_channelCount;
}


size_type TTAdpcmEncoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type TTAdpcmEncoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type TTAdpcmEncoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType TTAdpcmEncoder::getSampleType() const
{
	return m_sampleType;
}


size_type TTAdpcmEncoder::encode(const void** p_buffer,
                                 size_type    p_frames,
                                 size_type    p_offset)
{
	if (p_frames == 0)
	{
		return 0;
	}
	
	// create buffer large enough to hold encoded information
	size_t size = static_cast<size_t>(p_frames * m_channelCount);
	
	if (m_hasLeftOver)
	{
		++size;
	}
	
	// Chances are size is uneven, in that case, the last sample will be placed
	// in m_leftOver
	size /= 2;
	
	if (m_frameCount == 0)
	{
		// need to store per channel:
		// predictor (16 bit)
		// stepIndex (8 bit)
		// pad (8 bit)
		size += m_channelCount * 4;
	}
	
	fs::size_type fileSize = static_cast<fs::size_type>(size);
	u8* buffer = new u8[size];
	u8* scratch = buffer;
	
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			EncodeStates::size_type index = static_cast<EncodeStates::size_type>(channel);
			
			s16 sourceSample = reinterpret_cast<const s16**>(p_buffer)[channel][frame + p_offset];
			
			if (m_frameCount == 0)
			{
				// write Header
				m_encodeStates[index].predictor = static_cast<int>(sourceSample);
				m_encodeStates[index].stepIndex = 0;
				code::bufferutils::put(static_cast<s16>(m_encodeStates[index].predictor), scratch, size);
				code::bufferutils::put(static_cast<s8>(m_encodeStates[index].stepIndex), scratch, size);
				code::bufferutils::put(u8(0), scratch, size);
			}
			
			u8 sample = ttadpcm::encode(m_encodeStates[index], sourceSample);
			
			if (m_hasLeftOver)
			{
				m_leftOver |= static_cast<u8>(sample << 4);
				code::bufferutils::put(m_leftOver, scratch, size);
				m_hasLeftOver = false;
			}
			else
			{
				m_leftOver = sample;
				m_hasLeftOver = true;
			}
		}
		++m_frameCount;
	}
	
	m_file->write(buffer, fileSize - static_cast<tt::fs::size_type>(size));
	
	delete[] buffer;
	
	return p_frames;
}


size_type TTAdpcmEncoder::encodeInterleaved(const void* p_buffer,
                                            size_type   p_frames,
                                            size_type   p_offset)
{
	if (p_frames == 0)
	{
		return 0;
	}
	
	// create buffer large enough to hold encoded information
	size_t size = static_cast<size_t>(p_frames * m_channelCount);
	
	if (m_hasLeftOver)
	{
		++size;
	}
	
	// Chances are size is uneven, in that case, the last sample will be placed
	// in m_leftOver
	size /= 2;
	
	if (m_frameCount == 0)
	{
		// need to store per channel:
		// predictor (16 bit)
		// stepIndex (8 bit)
		// pad (8 bit)
		size += m_channelCount * 4;
	}
	
	fs::size_type fileSize = static_cast<fs::size_type>(size);
	u8* buffer = new u8[size];
	u8* scratch = buffer;
	
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			size_type index = ((frame + p_offset) * m_channelCount) + channel;
			s16 sourceSample = reinterpret_cast<const s16*>(p_buffer)[index];
			
			EncodeStates::size_type enindex = static_cast<EncodeStates::size_type>(channel);
			
			if (m_frameCount == 0)
			{
				// write Header
				m_encodeStates[enindex].predictor = static_cast<int>(sourceSample);
				m_encodeStates[enindex].stepIndex = 0;
				code::bufferutils::put(static_cast<s16>(m_encodeStates[enindex].predictor), scratch, size);
				code::bufferutils::put(static_cast<s8>(m_encodeStates[enindex].stepIndex), scratch, size);
				code::bufferutils::put(u8(0), scratch, size);
			}
			
			u8 sample = ttadpcm::encode(m_encodeStates[enindex], sourceSample);
			
			if (m_hasLeftOver)
			{
				m_leftOver |= static_cast<u8>(sample << 4);
				code::bufferutils::put(m_leftOver, scratch, size);
				m_hasLeftOver = false;
			}
			else
			{
				m_leftOver = sample;
				m_hasLeftOver = true;
			}
		}
		++m_frameCount;
	}
	
	m_file->write(buffer, fileSize - static_cast<tt::fs::size_type>(size));
	
	delete[] buffer;
	
	return p_frames;
}

// namespace end
}
}
}
}
