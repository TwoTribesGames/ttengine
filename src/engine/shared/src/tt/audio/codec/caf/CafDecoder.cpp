#include <cstdlib>

#include <tt/audio/codec/caf/CafDecoder.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace codec {
namespace caf {


CafDecoder::CafDecoder(const std::string& p_filename, fs::identifier p_fs)
:
Decoder(),
m_file(fs::open(p_filename, fs::OpenMode_Read, p_fs)),
m_rewindPosition(0),
m_position(0),
m_sampleType(SampleType_Float),
m_channelCount(0),
m_sampleRate(0),
m_sampleSize(0),
m_frameCount(0),
m_littleEndian(false)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	// get file information
	
	u8 header[8];
	m_file->read(header, 8);
	if (std::memcmp(header, "caff", 4) != 0)
	{
		TT_PANIC("'%s' is not a valid caf file.", p_filename.c_str());
		return;
	}
	
	const u8* scratch     = header + 4;
	size_t    scratchSize = 4;
	
	u16 fileVersion = code::bufferutils::be_get<u16>(scratch, scratchSize);
	if (fileVersion != 1)
	{
		TT_PANIC("'%s' has an unsupported caf version (%d), expected 1.", p_filename.c_str(), fileVersion);
		return;
	}
	
	u16 fileFlags = code::bufferutils::be_get<u16>(scratch, scratchSize);
	if (fileFlags != 0)
	{
		TT_WARN("'%s' has an unknown flags (%04X), ignoring.", p_filename.c_str(), fileFlags);
	}
	
	double sampleRate    = 0.0;
	u32 formatFlags      = 0;
	u32 bytesPerPacket   = 0;
	u32 framesPerPacket  = 0;
	u32 channelsPerFrame = 0;
	u32 bitsPerChannel   = 0;
	
	while (m_file->getPosition() < m_file->getLength())
	{
		// read chunk header
		u8 chunk[12] = {0};
		
		m_file->read(chunk, 12);
		
		scratch     = chunk + 4;
		scratchSize = 8;
		s64 size = code::bufferutils::be_get<s64>(scratch, scratchSize);
		
		if (std::memcmp(chunk, "desc", 4) == 0)
		{
			TT_ASSERTMSG(size == 32, "'%s' as a desc chunk with invalid size, found %d, expected 32.",
			             p_filename.c_str(), size);
			u8 desc[32] = {0};
			m_file->read(desc, 32);
			scratch = desc;
			scratchSize = 32;
			
			sampleRate = code::bufferutils::be_get<real64>(scratch, scratchSize);
			m_sampleRate = static_cast<size_type>(sampleRate);
			
			const char* formatID = reinterpret_cast<const char*>(scratch);
			scratch += 4;
			scratchSize -= 4;
			formatFlags      = code::bufferutils::be_get<u32>(scratch, scratchSize);
			bytesPerPacket   = code::bufferutils::be_get<u32>(scratch, scratchSize);
			framesPerPacket  = code::bufferutils::be_get<u32>(scratch, scratchSize);
			channelsPerFrame = code::bufferutils::be_get<u32>(scratch, scratchSize);
			bitsPerChannel   = code::bufferutils::be_get<u32>(scratch, scratchSize);
			
			if (std::memcmp(formatID, "lpcm", 4) == 0)
			{
				// Linear PCM
				if ((formatFlags & 1) != 0)
				{
					// floating point samples
					m_sampleType = SampleType_Float;
					m_sampleSize = 32;
				}
				else
				{
					switch (bitsPerChannel)
					{
					case 8:  m_sampleType = SampleType_Unsigned8; break;
					case 16: m_sampleType = SampleType_Signed16; break;
					default:
						TT_PANIC("'%s' has an unsupported sample size of %d bits per sample.",
						         p_filename.c_str(), bitsPerChannel);
						return;
					}
					m_sampleSize = static_cast<size_type>(bitsPerChannel);
				}
				
				m_littleEndian = (formatFlags & 2) != 0;
				TT_ASSERTMSG(framesPerPacket == 1, "'%s' has an invalid frames per packet count, found %d, expected 1",
				             p_filename.c_str(), framesPerPacket);
				
			}
			else if (std::memcmp(formatID, "ima4", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using Apple IMA4, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "aac ", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MPEG4 AAC, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "MAC3", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MACE3, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "MAC6", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MACE6, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "ulaw", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using ULaw, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "alaw", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using ALaw, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, ".mp1", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MPEGLayer1, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, ".mp2", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MPEGLayer2, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, ".mp3", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using MPEGLayer3, this is not supported.", p_filename.c_str());
				return;
			}
			else if (std::memcmp(formatID, "alac", 4) == 0)
			{
				TT_PANIC("'%s' is encoded using Apple Lossless, this is not supported.", p_filename.c_str());
				return;
			}
			else
			{
				TT_PANIC("'%s' is encoded using an unknown format (%c%c%c%c), this is not supported.",
				         p_filename.c_str(), formatID[0], formatID[1], formatID[2], formatID[3]);
				return;
			}
			
			m_channelCount = static_cast<size_type>(channelsPerFrame);
		}
		else if (std::memcmp(chunk, "data", 4) == 0)
		{
			m_rewindPosition = m_file->getPosition();
			if (size == -1)
			{
				// undertermined number of samples
				size = static_cast<s64>(m_file->getLength() - m_file->getPosition());
			}
			
			m_frameCount = static_cast<size_type>((size / bytesPerPacket) * framesPerPacket);
			
			// all done
			break;
		}
		else
		{
			m_file->seek(static_cast<tt::fs::pos_type>(size), tt::fs::SeekPos_Cur);
		}
	}
}


CafDecoder::~CafDecoder()
{
}


size_type CafDecoder::getChannelCount() const
{
	return m_channelCount;
}


size_type CafDecoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type CafDecoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type CafDecoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType CafDecoder::getSampleType() const
{
	return m_sampleType;
}


size_type CafDecoder::decode(SampleType p_type,
                             void**     p_buffer,
                             size_type  p_channels,
                             size_type  p_frames,
                             size_type  p_offset)
{
	size_type todo = std::min(p_frames, m_frameCount - m_position);
	
	// calculate number of samples to read
	size_type size = (todo * m_channelCount) * (m_sampleSize / 8);
	
	u8* data = new u8[size];
	if (data == 0)
	{
		TT_PANIC("Error allocating %d bytes.", size);
		return size_type();
	}
	const u8* scratch = data;
	size_t scratchSize = static_cast<size_t>(size);
	
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
	switch (m_sampleType)
	{
	case SampleType_Unsigned8:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				u8 sample = *scratch;
				++scratch;
				--scratchSize;
				
				if (channel >= p_channels)
				{
					continue;
				}
				
				switch (p_type)
				{
				case SampleType_Signed8:
					reinterpret_cast<s8**>(p_buffer)[channel][frame + p_offset] = static_cast<s8>(sample - 128);
					break;
					
				case SampleType_Unsigned8:
					reinterpret_cast<u8**>(p_buffer)[channel][frame + p_offset] = sample;
					break;
					
				case SampleType_Signed16:
					reinterpret_cast<s16**>(p_buffer)[channel][frame + p_offset] = static_cast<s16>((sample - 128) * 256);
					break;
					
				case SampleType_Float:
					reinterpret_cast<float**>(p_buffer)[channel][frame + p_offset] = static_cast<float>(sample - 128.0f) / 256.0f;
					break;
					
				default:
					break;
				}
			}
		}
		break;
		
	case SampleType_Signed16:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				s16 sample = m_littleEndian ?
				             (code::bufferutils::get<s16>(scratch, scratchSize)) :
				             (code::bufferutils::be_get<s16>(scratch, scratchSize));
				
				if (channel >= p_channels)
				{
					continue;
				}
				
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
		break;
		
	case SampleType_Float:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				real smp = m_littleEndian ?
				             (code::bufferutils::get<real>(scratch, scratchSize)) :
				             (code::bufferutils::be_get<real>(scratch, scratchSize));
				
				if (channel >= p_channels)
				{
					continue;
				}
				
				switch (p_type)
				{
				case SampleType_Signed8:
					reinterpret_cast<s8**>(p_buffer)[channel][frame + p_offset] = static_cast<s8>((smp) * 127.5f);
					break;
					
				case SampleType_Unsigned8:
					reinterpret_cast<u8**>(p_buffer)[channel][frame + p_offset] = static_cast<u8>((smp + 1.0f) * 127.5f);
					break;
					
				case SampleType_Signed16:
					reinterpret_cast<s16**>(p_buffer)[channel][frame + p_offset] = static_cast<s16>(smp * 32767.0f);
					break;
					
				case SampleType_Float:
					reinterpret_cast<float**>(p_buffer)[channel][frame + p_offset] = smp;
					break;
					
				default:
					break;
				}
			}
		}
		break;
		
	default:
		break;
	}
	
	delete[] data;
	
	return todo;
}


size_type CafDecoder::decodeInterleaved(SampleType p_type,
                                        void*      p_buffer,
                                        size_type  p_channels,
                                        size_type  p_frames,
                                        size_type  p_offset)
{
	size_type todo = std::min(p_frames, m_frameCount - m_position);
	
	// calculate number of samples to read
	size_type size = (todo * m_channelCount) * (m_sampleSize / 8);
	
	u8* data = new u8[size];
	if (data == 0)
	{
		TT_PANIC("Error allocating %d bytes.", size);
		return size_type();
	}
	const u8* scratch = data;
	size_t scratchSize = static_cast<size_t>(size);
	
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
	switch (m_sampleType)
	{
	case SampleType_Unsigned8:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				u8 sample = *scratch;
				++scratch;
				--scratchSize;
				
				if (channel >= p_channels)
				{
					continue;
				}
				
				size_type destIndex = ((frame + p_offset) * p_channels) + channel;
				
				switch (p_type)
				{
				case SampleType_Signed8:
					reinterpret_cast<s8*>(p_buffer)[destIndex] = static_cast<s8>(sample - 128);
					break;
					
				case SampleType_Unsigned8:
					reinterpret_cast<u8*>(p_buffer)[destIndex] = sample;
					break;
					
				case SampleType_Signed16:
					reinterpret_cast<s16*>(p_buffer)[destIndex] = static_cast<s16>((sample - 128) * 256);
					break;
					
				case SampleType_Float:
					reinterpret_cast<float*>(p_buffer)[destIndex] = static_cast<float>(sample - 128.0f) / 256.0f;
					break;
					
				default:
					break;
				}
			}
		}
		break;
		
	case SampleType_Signed16:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				s16 sample = m_littleEndian ?
				             (code::bufferutils::get<s16>(scratch, scratchSize)) :
				             (code::bufferutils::be_get<s16>(scratch, scratchSize));
				
				if (channel >= p_channels)
				{
					continue;
				}
				
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
		break;
		
	case SampleType_Float:
		for (size_type frame = 0; frame < todo; ++frame)
		{
			for (size_type channel = 0; channel < m_channelCount; ++channel)
			{
				real smp = m_littleEndian ?
				             (code::bufferutils::get<real>(scratch, scratchSize)) :
				             (code::bufferutils::be_get<real>(scratch, scratchSize));
				
				if (channel >= p_channels)
				{
					continue;
				}
				
				size_type destIndex = ((frame + p_offset) * p_channels) + channel;
				
				switch (p_type)
				{
				case SampleType_Signed8:
					reinterpret_cast<s8*>(p_buffer)[destIndex] = static_cast<s8>((smp) * 127.5f);
					break;
					
				case SampleType_Unsigned8:
					reinterpret_cast<u8*>(p_buffer)[destIndex] = static_cast<u8>((smp + 1.0f) * 127.5f);
					break;
					
				case SampleType_Signed16:
					reinterpret_cast<s16*>(p_buffer)[destIndex] = static_cast<s16>(smp * 32767.0f);
					break;
					
				case SampleType_Float:
					reinterpret_cast<float*>(p_buffer)[destIndex] = smp;
					break;
					
				default:
					break;
				}
			}
		}
		break;
		
	default:
		break;
	}
	
	delete[] data;
	
	return todo;
}


bool CafDecoder::rewind()
{
	if (m_file->getPosition() != static_cast<fs::pos_type>(m_file->getLength()))
	{
		TT_PANIC("Not at end of file.");
		return false;
	}
	m_file->seek(m_rewindPosition, fs::SeekPos_Set);
	m_position = 0;
	return true;
}

// namespace end
}
}
}
}
