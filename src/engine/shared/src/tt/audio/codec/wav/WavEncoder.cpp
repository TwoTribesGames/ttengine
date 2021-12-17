#include <cstring>

#include <tt/audio/codec/wav/WavEncoder.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

WavEncoder::WavEncoder(SampleType         p_type,
                       size_type          p_channelCount,
                       size_type          p_sampleRate,
                       const std::string& p_filename,
                       fs::identifier     p_fs)
:
Encoder(),
m_file(fs::open(p_filename, fs::OpenMode_Write, p_fs)),
m_sampleType(p_type),
m_channelCount(p_channelCount),
m_sampleRate(p_sampleRate),
m_sampleSize(0),
m_frameCount(0)
{
	switch (p_type)
	{
	case SampleType_Unsigned8:
		m_sampleSize = 8;
		break;
		
	case SampleType_Signed16:
		m_sampleSize = 16;
		break;
		
	case SampleType_Float:
		m_sampleSize = 32;
		break;
		
	default:
		TT_PANIC("Unknown/unsupported sample type %d.", p_type);
		return;
	}
	
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	// write file information
	m_file->write("RIFF", 4);
	
	// dummy size value
	u32 filesize = 0;
	fs::writeInteger(m_file, filesize);
	
	m_file->write("WAVE", 4);
	
	// Write format chunk
	m_file->write("fmt ", 4);
	
	s32 fmtsize = 16;
	fs::writeInteger(m_file, fmtsize);
	
	s16 format;
	if (m_sampleType == SampleType_Float)
	{
		format = 3;
	}
	else
	{
		format = 1;
	}
	fs::writeInteger(m_file, format);
	
	u16 channels = static_cast<u16>(m_channelCount);
	fs::writeInteger(m_file, channels);
	
	u32 samplesPerSec = static_cast<u32>(m_sampleRate);
	fs::writeInteger(m_file, samplesPerSec);
	
	u32 avgBytesPerSec = static_cast<u32>(m_sampleRate * m_channelCount * (m_sampleSize / 8));
	fs::writeInteger(m_file, avgBytesPerSec);
	
	u16 blockAlign = static_cast<u16>(m_channelCount * (m_sampleSize / 8));
	fs::writeInteger(m_file, blockAlign);
	
	u16 bitsPerSample = static_cast<u16>(m_sampleSize);
	fs::writeInteger(m_file, bitsPerSample);
	
	// write data chunk
	m_file->write("data", 4);
	
	// dummy size value
	s32 datasize = 0;
	fs::writeInteger(m_file, datasize);
	
	// data follows
}


WavEncoder::~WavEncoder()
{
	if (m_file == 0)
	{
		// file not open, error in constructor
		return;
	}
	
	// finalize
	size_type frameSize = m_channelCount * (m_sampleSize / 8);
	
	size_type fmtSize  = 8 + 16; // header + data
	size_type dataSize = 8 + (m_frameCount * frameSize); // header + data
	size_type fileSize = fmtSize + dataSize;
	
	m_file->seek(4, fs::SeekPos_Set); // 4 bytes from begin, file size
	u32 headerFileSize = static_cast<u32>(fileSize);
	fs::writeInteger(m_file, headerFileSize);
	
	m_file->seek(40, fs::SeekPos_Set); // 40 bytes from begin, data size
	s32 headerDataSize = static_cast<s32>(m_frameCount * frameSize);
	fs::writeInteger(m_file, headerDataSize);
	
	// all done
}


size_type WavEncoder::getChannelCount() const
{
	return m_channelCount;
}


size_type WavEncoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type WavEncoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type WavEncoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType WavEncoder::getSampleType() const
{
	return m_sampleType;
}


size_type WavEncoder::encode(const void** p_buffer,
                             size_type    p_frames,
                             size_type    p_offset)
{
	if (p_frames == 0)
	{
		return 0;
	}
	
	// create buffer large enough to hold encoded information
	size_t size = static_cast<size_t>(p_frames * m_channelCount * (m_sampleSize / 8));
	fs::size_type fileSize = static_cast<fs::size_type>(size);
	u8* buffer = new u8[size];
	u8* scratch = buffer;
	
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			switch (m_sampleType)
			{
			case SampleType_Signed8:
				{
					s8 sample = reinterpret_cast<const s8**>(p_buffer)[channel][frame + p_offset];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Unsigned8:
				{
					u8 sample = reinterpret_cast<const u8**>(p_buffer)[channel][frame + p_offset];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Signed16:
				{
					s16 sample = reinterpret_cast<const s16**>(p_buffer)[channel][frame + p_offset];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Float:
				{
					float sample = reinterpret_cast<const float**>(p_buffer)[channel][frame + p_offset];
					/*
					u32 sampleInt = *reinterpret_cast<u32*>(&sample);
					code::bufferutils::put(sampleInt, scratch, size);
					*/
					code::bufferutils::put(real(sample), scratch, size);
				}
				break;
				
			default:
				break;
			}
		}
		++m_frameCount;
	}
	
	m_file->write(buffer, fileSize);
	
	delete[] buffer;
	
	return p_frames;
}


size_type WavEncoder::encodeInterleaved(const void* p_buffer,
                                        size_type   p_frames,
                                        size_type   p_offset)
{
	if (p_frames == 0)
	{
		return 0;
	}
	
	// create buffer large enough to hold encoded information
	size_t size = static_cast<size_t>(p_frames * m_channelCount * (m_sampleSize / 8));
	fs::size_type fileSize = static_cast<fs::size_type>(size);
	u8* buffer = new u8[size];
	u8* scratch = buffer;
	
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			size_type index = ((frame + p_offset) * m_channelCount) + channel;
			
			switch (m_sampleType)
			{
			case SampleType_Signed8:
				{
					s8 sample = reinterpret_cast<const s8*>(p_buffer)[index];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Unsigned8:
				{
					u8 sample = reinterpret_cast<const u8*>(p_buffer)[index];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Signed16:
				{
					s16 sample = reinterpret_cast<const s16*>(p_buffer)[index];
					code::bufferutils::put(sample, scratch, size);
				}
				break;
				
			case SampleType_Float:
				{
					float sample = reinterpret_cast<const float*>(p_buffer)[index];
					/*
					u32 sampleInt = *reinterpret_cast<u32*>(&sample);
					code::bufferutils::put(sampleInt, scratch, size);
					*/
					code::bufferutils::put(real(sample), scratch, size);
				}
				break;
				
			default:
				break;
			}
		}
		++m_frameCount;
	}
	
	m_file->write(buffer, fileSize);
	
	delete[] buffer;
	
	return p_frames;
}

// namespace end
}
}
}
}
