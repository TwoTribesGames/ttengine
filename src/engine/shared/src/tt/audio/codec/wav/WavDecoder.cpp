#include <cstring>
#include <algorithm>

#include <tt/audio/codec/SampleConverter.h>
#include <tt/audio/codec/wav/WavDecoder.h>
#include <tt/audio/codec/wav/WavFormatDecoderImaAdpcm.h>
#include <tt/audio/codec/wav/WavFormatDecoderPcm.h>
#include <tt/fs/File.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

enum WaveFormat
{
	WaveFormat_Unknown             = 0x0000,
	WaveFormat_PCM                 = 0x0001,
	WaveFormat_ADPCM               = 0x0002, // Microsoft ADPCM
	WaveFormat_Float               = 0x0003,
	WaveFormat_IBM_CVSD            = 0x0005,
	WaveFormat_ALAW                = 0x0006,
	WaveFormat_MULAW               = 0x0007,
	WaveFormat_OKI_ADPCM           = 0x0010,
	WaveFormat_IMA_ADPCM           = 0x0011, // == WaveFormat_DVIADPCM
	WaveFormat_DVI_ADPCM           = 0x0011,
	WaveFormat_MEDIASPACE_ADPCM    = 0x0012,
	WaveFormat_SIERRA_ADPCM        = 0x0013,
	WaveFormat_G723_ADPCM          = 0x0014,
	WaveFormat_DIGISTD             = 0x0015,
	WaveFormat_DIGIFX              = 0x0016,
	WaveFormat_YAMAHA_ADPCM        = 0x0020,
	WaveFormat_SONARCH             = 0x0021,
	WaveFormat_DSPGROUP_TRUESPEECH = 0x0022,
	WaveFormat_ECHOSC1             = 0x0023,
	WaveFormat_AUDIOFILE_AF36      = 0x0024,
	WaveFormat_APTX                = 0x0025,
	WaveFormat_AUDIOFILE_AF10      = 0x0026,
	WaveFormat_DOLBY_AC2           = 0x0030,
	WaveFormat_GSM610              = 0x0031,
	WaveFormat_ANTEX_ADPCME        = 0x0033,
	WaveFormat_CONTROL_RES_VQLPC   = 0x0034,
	WaveFormat_DIGI_REAL           = 0x0035,
	WaveFormat_DIGI_ADPCM          = 0x0036,
	WaveFormat_CONTROL_RES_CR10    = 0x0037,
	WaveFormat_NMS_VBXADPCM        = 0x0038,
	WaveFormat_G721_ADPCM          = 0x0040,
	WaveFormat_MPEG                = 0x0050,
	WaveFormat_CREATIVE_ADPCM      = 0x0200,
	WaveFormat_FM_TOWNS_SND        = 0x0300,
	WaveFormat_OLIGSM              = 0x1000,
	WaveFormat_OLIADPCM            = 0x1001,
	WaveFormat_OLICELP             = 0x1002,
	WaveFormat_OLISBC              = 0x1003,
	WaveFormat_OLIOPR              = 0x1004
};


WavDecoder::WavDecoder(const std::string& p_filename, fs::identifier p_fs)
:
Decoder(),
m_file(fs::open(p_filename, fs::OpenMode_Read, p_fs)),
m_format(),
m_decoder(0),
m_buffer(0),
m_buffers(0),
m_bufferSize(0),
m_bufferPosition(0)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	// get file information
	
	char riff[4];
	m_file->read(riff, 4);
	if (std::memcmp(riff, "RIFF", 4) != 0)
	{
		TT_PANIC("'%s' is not a valid wave file.", p_filename.c_str());
		return;
	}
	
	u32 filesize = 0;
	fs::readInteger(m_file, &filesize);
	
	char wave[4];
	m_file->read(wave, 4);
	if (std::memcmp(wave, "WAVE", 4) != 0)
	{
		TT_WARN("'%s' is not a valid wave file.", p_filename.c_str());
		return;
	}
	
	u16 blockAlign = 0;
	
	bool found = false;
	while (found == false)
	{
		char chunkID[4];
		m_file->read(chunkID, 4);
		
		s32 chunkSize = 0;
		tt::fs::readInteger(m_file, &chunkSize);
		
		if (std::memcmp(chunkID, "fmt ", 4) == 0)
		{
			// parse format
			s16 format = 0;
			tt::fs::readInteger(m_file, &format);
			
			u16 channels = 0;
			tt::fs::readInteger(m_file, &channels);
			
			u32 samplesPerSec = 0;
			tt::fs::readInteger(m_file, &samplesPerSec);
			
			u32 avgBytesPerSec = 0;
			tt::fs::readInteger(m_file, &avgBytesPerSec);
			
			tt::fs::readInteger(m_file, &blockAlign);
			
			u16 bitsPerSample = 0;
			tt::fs::readInteger(m_file, &bitsPerSample);
			
			m_format.channelCount   = static_cast<size_type>(channels);
			m_format.samplesPerSec  = static_cast<size_type>(samplesPerSec);
			m_format.avgBytesPerSec = static_cast<size_type>(avgBytesPerSec);
			m_format.sampleSize     = static_cast<size_type>(bitsPerSample);
			m_format.blockSize      = static_cast<size_type>(blockAlign);
			m_format.format         = static_cast<size_type>(format);
			
			switch (format)
			{
			case WaveFormat_PCM:
				m_decoder = new WavFormatDecoderPcm(m_format, m_file);
				break;
				
			case WaveFormat_IMA_ADPCM:
				m_decoder = new WavFormatDecoderImaAdpcm(m_format, m_file);
				break;
				
			default:
				TT_WARN("Unsupported wave type %d\n", format);
				return;
			}
			
			if (chunkSize > 16)
			{
				s16 extraSize = 0;
				fs::readInteger(m_file, &extraSize);
				m_format.extraSize = static_cast<size_type>(extraSize);
				
				fs::pos_type position = m_file->getPosition();
				
				m_decoder->parseExFormat(m_format.extraSize);
				
				m_file->seek(position + static_cast<fs::pos_type>(extraSize), fs::SeekPos_Set);
			}
		}
		else if (std::memcmp(chunkID, "fact", 4) == 0)
		{
			fs::pos_type position = m_file->getPosition();
			
			m_decoder->parseFact(static_cast<size_type>(chunkSize));
			
			m_file->seek(position + static_cast<fs::pos_type>(chunkSize), fs::SeekPos_Set);
		}
		else if (std::memcmp(chunkID, "data", 4) == 0)
		{
			// parse data
			m_decoder->parseData(static_cast<size_type>(chunkSize));
			
			found = true;
		}
		else
		{
			// unknown chunk, skip it
			m_file->seek(static_cast<tt::fs::pos_type>(chunkSize), tt::fs::SeekPos_Cur);
		}
	}
	
	size_type blockSize = m_decoder->getBlockBufferSize();
	
	m_buffer = ::operator new(static_cast<size_t>(blockSize));
	if (m_buffer == 0)
	{
		TT_PANIC("Failed to allocate %d bytes.", blockSize);
		return;
	}
	
	m_buffers = new void*[m_format.channelCount];
	if (m_buffers == 0)
	{
		TT_PANIC("Failed to allocate %d bytes.", sizeof(void*) * m_format.channelCount);
		return;
	}
	
	u8* buffer = reinterpret_cast<u8*>(m_buffer);
	for (size_type i = 0; i < m_format.channelCount; ++i)
	{
		m_buffers[i] = buffer;
		buffer += blockSize / m_format.channelCount;
	}
}


WavDecoder::~WavDecoder()
{
	delete m_decoder;
	::operator delete(m_buffer);
	delete[] m_buffers;
}


size_type WavDecoder::getChannelCount() const
{
	return m_format.channelCount;
}


size_type WavDecoder::getSampleRate() const
{
	return m_format.samplesPerSec;
}


size_type WavDecoder::getSampleSize() const
{
	return m_format.sampleSize;
}


size_type WavDecoder::getFrameCount() const
{
	return m_decoder->getFrameCount();
}


SampleType WavDecoder::getSampleType() const
{
	return m_decoder->getSampleType();
}


size_type WavDecoder::decode(SampleType p_type,
                             void**     p_buffer,
                             size_type  p_channels,
                             size_type  p_frames,
                             size_type  p_offset)
{
	// see if we have anything cached
	size_type framesDecoded = 0;
	size_type channels = std::min(p_channels, m_format.channelCount);
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	SampleConverter& converter = getConverter(m_decoder->getSampleType(), p_type);
	
	while (m_bufferPosition < m_bufferSize && p_frames > 0)
	{
		// copy cached frames to p_buffer
		for (size_type i = 0; i < channels; ++i)
		{
			converter.convert(m_buffers, p_buffer, i, m_bufferPosition, framesDecoded + p_offset);
		}
		++framesDecoded;
		++m_bufferPosition;
		--p_frames;
	}
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	// calculate the amount of full blocks we can read
	size_type blockCount = p_frames / m_decoder->getFramesPerBlock();
	if (blockCount > 0)
	{
		// create some buffers
		size_type blockSize = m_decoder->getBlockBufferSize() * blockCount;
		void* buffer = ::operator new(static_cast<size_t>(blockSize));
		if (buffer == 0)
		{
			TT_PANIC("Unable to allocate %d bytes.\n", blockSize);
			
			// TODO: fall back to processing 1 block each time
			return framesDecoded;
		}
		
		void** buffers = new void*[m_format.channelCount];
		u8* scratch = reinterpret_cast<u8*>(buffer);
		for (size_type i = 0; i < m_format.channelCount; ++i)
		{
			buffers[i] = scratch;
			scratch += blockSize / m_format.channelCount;
		}
		
		// let the decoder decode the blocks
		size_type frames = m_decoder->decodeBlocks(buffers, blockCount);
		
		// copy the decoded blocks
		for (size_type frame = 0; frame < frames; ++frame)
		{
			for (size_type channel = 0; channel < channels; ++channel)
			{
				converter.convert(buffers, p_buffer, channel, frame, framesDecoded + p_offset);
			}
			++framesDecoded;
			--p_frames;
		}
		
		// clean up the buffers
		delete[] buffers;
		::operator delete(buffer);
	}
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	// decode a single block (last one we need) to the internal buffer
	m_bufferSize = m_decoder->decodeBlocks(m_buffers, 1);
	m_bufferPosition = 0;
	
	while (m_bufferPosition < m_bufferSize && p_frames > 0)
	{
		// copy cached frames to p_buffer
		for (size_type i = 0; i < channels; ++i)
		{
			converter.convert(m_buffers, p_buffer, i, m_bufferPosition, framesDecoded + p_offset);
		}
		++framesDecoded;
		++m_bufferPosition;
		--p_frames;
	}
	
	return framesDecoded;
}


size_type WavDecoder::decodeInterleaved(SampleType p_type,
                                        void*      p_buffer,
                                        size_type  p_channels,
                                        size_type  p_frames,
                                        size_type  p_offset)
{
	// see if we have anything cached
	size_type framesDecoded = 0;
	size_type channels = std::min(p_channels, m_format.channelCount);
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	SampleConverter& converter = getConverter(m_decoder->getSampleType(), p_type);
	
	while (m_bufferPosition < m_bufferSize && p_frames > 0)
	{
		// copy cached frames to p_buffer
		for (size_type i = 0; i < channels; ++i)
		{
			converter.convert(m_buffer, p_buffer, m_bufferPosition, ((framesDecoded + p_offset) * p_channels) + i);
			++m_bufferPosition;
		}
		for (size_type i = channels; i < m_format.channelCount; ++i)
		{
			++m_bufferPosition;
		}
		++framesDecoded;
		--p_frames;
	}
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	// calculate the amount of full blocks we can read
	size_type blockCount = p_frames / m_decoder->getFramesPerBlock();
	if (blockCount > 0)
	{
		// create some buffers
		size_type blockSize = m_decoder->getBlockBufferSize() * blockCount;
		void* buffer = ::operator new(static_cast<size_t>(blockSize));
		if (buffer == 0)
		{
			TT_PANIC("Unable to allocate %d bytes.\n", blockSize);
			
			// TODO: fall back to processing 1 block each time
			return framesDecoded;
		}
		
		// let the decoder decode the blocks
		size_type frames = m_decoder->decodeBlocksInterleaved(buffer, blockCount);
		
		size_type bufferPosition = 0;
		// copy the decoded blocks
		for (size_type frame = 0; frame < frames; ++frame)
		{
			for (size_type channel = 0; channel < channels; ++channel)
			{
				converter.convert(buffer,
				                  p_buffer,
				                  bufferPosition,
				                  ((framesDecoded + p_offset) * p_channels) + channel);
				++bufferPosition;
			}
			for (size_type i = channels; i < m_format.channelCount; ++i)
			{
				++bufferPosition;
			}
			++framesDecoded;
			--p_frames;
		}
		
		// clean up the buffers
		::operator delete(buffer);
	}
	
	if (p_frames == 0)
	{
		return framesDecoded;
	}
	
	// decode a single block (last one we need) to the internal buffer
	m_bufferSize = m_decoder->decodeBlocks(m_buffers, 1);
	m_bufferPosition = 0;
	
	while (m_bufferPosition < m_bufferSize && p_frames > 0)
	{
		// copy cached frames to p_buffer
		for (size_type i = 0; i < channels; ++i)
		{
			converter.convert(m_buffer, p_buffer, m_bufferPosition, ((framesDecoded + p_offset) * p_channels) + i);
			++m_bufferPosition;
		}
		for (size_type i = channels; i < m_format.channelCount; ++i)
		{
			++m_bufferPosition;
		}
		++framesDecoded;
		--p_frames;
	}
	
	return framesDecoded;
}


bool WavDecoder::rewind()
{
	return m_decoder->rewind();
}

// namespace end
}
}
}
}
