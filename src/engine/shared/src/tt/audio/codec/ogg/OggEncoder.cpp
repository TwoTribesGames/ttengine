#include <cstring>
#include <cstdlib>
#include <ctime>

#include <tt/audio/codec/ogg/OggEncoder.h>
#include <tt/fs/File.h>

#include <vorbis/vorbisenc.h>


namespace tt {
namespace audio {
namespace codec {
namespace ogg {

OggEncoder::OggEncoder(float              p_quality,
                       size_type          p_channelCount,
                       size_type          p_sampleRate,
                       const std::string& p_filename,
                       fs::identifier     p_fs)
:
Encoder(),
m_file(fs::open(p_filename, fs::OpenMode_Write, p_fs)),
m_sampleType(SampleType_Float),
m_vorbisInfo(),
m_vorbisComment(),
m_vorbisDspState(),
m_vorbisBlock(),
m_oggStreamState(),
m_oggPage(),
m_oggPacket(),
m_channelCount(p_channelCount),
m_sampleRate(p_sampleRate),
m_sampleSize(32),
m_frameCount(0)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return;
	}
	
	std::memset(&m_vorbisInfo,     0, sizeof(vorbis_info));
	std::memset(&m_vorbisComment,  0, sizeof(vorbis_comment));
	std::memset(&m_vorbisDspState, 0, sizeof(vorbis_dsp_state));
	std::memset(&m_vorbisBlock,    0, sizeof(vorbis_block));
	
	std::memset(&m_oggStreamState, 0, sizeof(ogg_stream_state));
	std::memset(&m_oggPage,        0, sizeof(ogg_page));
	std::memset(&m_oggPacket,      0, sizeof(ogg_packet));
	
	vorbis_info_init(&m_vorbisInfo);
	
	// choose an encoding mode.
	if (vorbis_encode_init_vbr(&m_vorbisInfo,
	                           static_cast<long>(m_channelCount),
	                           static_cast<long>(m_sampleRate),
	                           p_quality) != 0)
	{
		TT_PANIC("Unable to initialize encoder.");
		return;
	}
	
	// add a comment.
	vorbis_comment_init(&m_vorbisComment);
	char vorbisTag[]      = "ENCODER";
	char vorbisContents[] = "Two Tribes Ogg Vorbis Encoder";
	vorbis_comment_add_tag(&m_vorbisComment, vorbisTag, vorbisContents);
	
	// set up the analysis state and auxiliary encoding storage.
	if (vorbis_analysis_init(&m_vorbisDspState, &m_vorbisInfo) != 0)
	{
		TT_PANIC("Unable to initialize vorbis analysis.");
		return;
	}
	if (vorbis_block_init(&m_vorbisDspState, &m_vorbisBlock) != 0)
	{
		TT_PANIC("Unable to initialize vorbis block.");
	}
	
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	if (ogg_stream_init(&m_oggStreamState, std::rand()) != 0)
	{
		TT_PANIC("Unable to initialize ogg stream.");
		return;
	}
	
	// Vorbis streams begin with three headers; the initial header (with
	// most of the codec setup parameters) which is mandated by the Ogg
	// bitstream spec.  The second header holds any comment fields. The
	// third header holds the bitstream codebook.  We merely need to
	// make the headers, then pass them to libvorbis one at a time;
	// libvorbis handles the additional Ogg bitstream constraints 
	
	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;
		
		vorbis_analysis_headerout(&m_vorbisDspState,
		                          &m_vorbisComment,
		                          &header,
		                          &header_comm,
		                          &header_code);
		ogg_stream_packetin(&m_oggStreamState, &header);
		ogg_stream_packetin(&m_oggStreamState, &header_comm);
		ogg_stream_packetin(&m_oggStreamState, &header_code);
		
		// This ensures the actual audio data will start on a new page, as per spec
		for (;;)
		{
			if(ogg_stream_flush(&m_oggStreamState, &m_oggPage) == 0)
			{
				// no data flushed from stream to page
				break;
			}
			
			m_file->write(m_oggPage.header, static_cast<fs::size_type>(m_oggPage.header_len));
			m_file->write(m_oggPage.body, static_cast<fs::size_type>(m_oggPage.body_len));
		}
	}
	
	// ready for encoding.
}


OggEncoder::~OggEncoder()
{
	vorbis_analysis_wrote(&m_vorbisDspState, 0);
	
	while (vorbis_analysis_blockout(&m_vorbisDspState, &m_vorbisBlock) == 1)
	{
		// analysis, assume we want to use bitrate management
		vorbis_analysis(&m_vorbisBlock, NULL);
		vorbis_bitrate_addblock(&m_vorbisBlock);
		
		while (vorbis_bitrate_flushpacket(&m_vorbisDspState, &m_oggPacket) != 0)
		{
			// weld the packet into the bitstream
			ogg_stream_packetin(&m_oggStreamState, &m_oggPacket);
			
			// write out pages (if any)
			for (;;)
			{
				if (ogg_stream_pageout(&m_oggStreamState, &m_oggPage) == 0)
				{
					break;
				}
				
				m_file->write(m_oggPage.header, static_cast<fs::size_type>(m_oggPage.header_len));
				m_file->write(m_oggPage.body, static_cast<fs::size_type>(m_oggPage.body_len));
				
				if (ogg_page_eos(&m_oggPage) != 0)
				{
					break;
				}
			}
		}
	}
	
	ogg_stream_clear(&m_oggStreamState);
	vorbis_block_clear(&m_vorbisBlock);
	vorbis_dsp_clear(&m_vorbisDspState);
	vorbis_comment_clear(&m_vorbisComment);
	vorbis_info_clear(&m_vorbisInfo);
}


size_type OggEncoder::getChannelCount() const
{
	return m_channelCount;
}


size_type OggEncoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type OggEncoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type OggEncoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType OggEncoder::getSampleType() const
{
	return m_sampleType;
}



size_type OggEncoder::encode(const void** p_buffer,
                             size_type    p_frames,
                             size_type    p_offset)
{
	if (p_frames == 0)
	{
		return p_frames;
	}
	
	// expose the buffer to submit data
	float** destBuffer = vorbis_analysis_buffer(&m_vorbisDspState, static_cast<int>(p_frames));
	const float** srcBuffer = reinterpret_cast<const float**>(p_buffer);
	
	// copy buffer
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			destBuffer[channel][frame] = srcBuffer[channel][frame + p_offset];
		}
	}
	
	// tell the library how much we actually submitted
	vorbis_analysis_wrote(&m_vorbisDspState, p_frames);
	
	while (vorbis_analysis_blockout(&m_vorbisDspState, &m_vorbisBlock) == 1)
	{
		// analysis, assume we want to use bitrate management
		vorbis_analysis(&m_vorbisBlock, NULL);
		vorbis_bitrate_addblock(&m_vorbisBlock);
		
		while (vorbis_bitrate_flushpacket(&m_vorbisDspState, &m_oggPacket))
		{
			// weld the packet into the bitstream
			ogg_stream_packetin(&m_oggStreamState, &m_oggPacket);
			
			// write out pages (if any)
			for (;;)
			{
				if (ogg_stream_pageout(&m_oggStreamState, &m_oggPage) == 0)
				{
					break;
				}
				
				m_file->write(m_oggPage.header, static_cast<fs::size_type>(m_oggPage.header_len));
				m_file->write(m_oggPage.body, static_cast<fs::size_type>(m_oggPage.body_len));
				
				if (ogg_page_eos(&m_oggPage))
				{
					break;
				}
			}
		}
	}
	return p_frames;
}


size_type OggEncoder::encodeInterleaved(const void* p_buffer,
                                        size_type   p_frames,
                                        size_type   p_offset)
{
	if (p_frames == 0)
	{
		return p_frames;
	}
	
	// expose the buffer to submit data
	float** destBuffer = vorbis_analysis_buffer(&m_vorbisDspState, static_cast<int>(p_frames));
	const float* srcBuffer = reinterpret_cast<const float*>(p_buffer);
	
	// copy buffer
	size_type index = p_offset * m_channelCount;
	for (size_type frame = 0; frame < p_frames; ++frame)
	{
		for (size_type channel = 0; channel < m_channelCount; ++channel)
		{
			destBuffer[channel][frame] = srcBuffer[index];
			++index;
		}
	}
	
	// tell the library how much we actually submitted
	vorbis_analysis_wrote(&m_vorbisDspState, static_cast<int>(p_frames));
	
	while (vorbis_analysis_blockout(&m_vorbisDspState, &m_vorbisBlock) == 1)
	{
		// analysis, assume we want to use bitrate management
		vorbis_analysis(&m_vorbisBlock, NULL);
		vorbis_bitrate_addblock(&m_vorbisBlock);
		
		while (vorbis_bitrate_flushpacket(&m_vorbisDspState, &m_oggPacket) != 0)
		{
			// weld the packet into the bitstream
			ogg_stream_packetin(&m_oggStreamState, &m_oggPacket);
			
			// write out pages (if any)
			for (;;)
			{
				if (ogg_stream_pageout(&m_oggStreamState, &m_oggPage) == 0)
				{
					break;
				}
				
				m_file->write(m_oggPage.header, static_cast<fs::size_type>(m_oggPage.header_len));
				m_file->write(m_oggPage.body, static_cast<fs::size_type>(m_oggPage.body_len));
				
				if (ogg_page_eos(&m_oggPage))
				{
					break;
				}
			}
		}
	}
	return p_frames;
}

// namespace end
}
}
}
}
