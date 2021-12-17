#include <cstring>

#include <tt/audio/codec/ogg/OggDecoder.h>
#include <tt/fs/File.h>
#include <tt/mem/util.h>


namespace tt {
namespace audio {
namespace codec {
namespace ogg {

enum { BufferSize = 4096 };


//--------------------------------------------------------------------------------------------------
// Public member functions

OggDecoder::OggDecoder(const std::string& p_filename, fs::identifier p_fs)
:
Decoder(),
m_file(fs::open(p_filename, fs::OpenMode_Read, p_fs)),
m_syncState(),
m_streamState(),
m_page(),
m_packet(),
m_info(),
m_comment(),
m_dspState(),
m_block(),
m_buffer(0),
m_framesPerBuffer(0),
m_decodeBuffer(0),
m_eos(false),
m_samplesReady(0),
m_samplesStart(0),
m_sampleType(SampleType_Float),
m_channelCount(0),
m_sampleRate(0),
m_sampleSize(0),
m_frameCount(0)
{
	if (m_file == 0)
	{
		TT_PANIC("Unable to open file '%s'.", p_filename.c_str());
		return;
	}
	
	init();
}


OggDecoder::~OggDecoder()
{
	destroy();
}


size_type OggDecoder::getChannelCount() const
{
	return m_channelCount;
}


size_type OggDecoder::getSampleRate() const
{
	return m_sampleRate;
}


size_type OggDecoder::getSampleSize() const
{
	return m_sampleSize;
}


size_type OggDecoder::getFrameCount() const
{
	return m_frameCount;
}


SampleType OggDecoder::getSampleType() const
{
	return m_sampleType;
}


size_type OggDecoder::decode(SampleType p_type,
                             void**     p_buffer,
                             size_type  p_channels,
                             size_type  p_frames,
                             size_type  p_offset)
{
	TT_NULL_ASSERT(p_buffer);
	for (size_type i = 0; i < p_channels; ++i)
	{
		TT_NULL_ASSERT(p_buffer[i]);
	}
	
	size_type decoded = 0;
	
	size_type index = p_offset;
	
	if (p_frames == 0)
	{
		return p_frames;
	}
	
	while (p_frames > 0 && m_eos == false)
	{
		while (p_frames > 0 && m_eos == false)
		{
			int result = 1;
			if (m_samplesReady == 0)
			{
				result = ogg_sync_pageout(&m_syncState, &m_page);
			}
			
			if (result == 0)
			{
				// need more data
				break;
			}
			
			if (result < 0)
			{
				// missing or corrupt data at this page position
				TT_WARN("Corrupt or missing data in bitstream; continuing...\n");
			}
			else
			{
				if (m_samplesReady == 0)
				{
					ogg_stream_pagein(&m_streamState, &m_page); // can safely ignore errors at this point
				}
				for (;;)
				{
					if (m_samplesReady == 0)
					{
						result = ogg_stream_packetout(&m_streamState, &m_packet);
					}
					else
					{
						result = 1;
					}
					
					if (result == 0)
					{
						// need more data
						break;
					}
					
					if (result < 0)
					{
						// missing or corrupt data at this page position
						// no reason to complain; already complained above
					}
					else
					{
						// we have a packet. Decode it
						
						// test for success!
						
						if (m_samplesReady == 0)
						{
							if (vorbis_synthesis(&m_block, &m_packet) == 0)
							{
								vorbis_synthesis_blockin(&m_dspState, &m_block);
							}
						}
						
						// **pcm is a multichannel float vector.  In stereo, for
						// example, pcm[0] is left, and pcm[1] is right.  samples is
						// the size of each channel.  Convert the float values
						// (-1.<=range<=1.) to whatever PCM format and write it out
						
						while (m_samplesReady != 0 || (m_samplesReady = vorbis_synthesis_pcmout(&m_dspState, &m_decodeBuffer) ) > 0)
						{
							bool clipflag = false;
							int decodedSamples = (m_samplesReady < m_framesPerBuffer ? m_samplesReady : m_framesPerBuffer);
							
							for (int j = m_samplesStart; j < decodedSamples; ++j)
							{
								for (int i = 0; i < m_info.channels; i++)
								{
									float sample = m_decodeBuffer[i][j];
									
									// clamp
									if (sample < -1.0f)
									{
										sample = -1.0f;
										clipflag = true;
									}
									else if (sample > 1.0f)
									{
										sample = 1.0f;
										clipflag = true;
									}
									
									// convert to destination type
									if (i < p_channels)
									{
										switch (p_type)
										{
										case SampleType_Signed8:
											reinterpret_cast<s8**>(p_buffer)[i][index] = static_cast<s8>(sample * 127.0f);
											break;
											
										case SampleType_Unsigned8:
											reinterpret_cast<u8**>(p_buffer)[i][index] = static_cast<u8>((sample + 1.0f) * 127.5f);
											break;
											
										case SampleType_Signed16:
											reinterpret_cast<s16**>(p_buffer)[i][index] = static_cast<s16>(sample * 32767.0f);
											break;
											
										case SampleType_Float:
											reinterpret_cast<float**>(p_buffer)[i][index] = sample;
											break;
											
										default:
											break;
										}
									}
								}
								
								++index;
								++m_samplesStart;
								--p_frames;
								++decoded;
								
								if (p_frames == 0)
								{
									return decoded;
								}
							}
							m_samplesStart = 0;
							m_samplesReady = 0;
							
							if (clipflag)
							{
								// just ignore this
								//TT_WARN("Clipping in frame %lld\n", m_dspState.sequence);
							}
							
							// tell libvorbis how many samples we actually consumed
							vorbis_synthesis_read(&m_dspState, decodedSamples);
						}
					}
				}
			}
		}
		
		if (p_frames > 0)
		{
			// read the next part of data
			m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
			
			fs::size_type bytes = m_file->read(m_buffer, BufferSize);
			
			ogg_sync_wrote(&m_syncState, static_cast<long>(bytes));
			
			if (bytes == 0)
			{
				/*
				m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
				m_file->seekToBegin();
				bytes = m_file->read(m_buffer, BufferSize);
				ogg_sync_wrote(&m_syncState, static_cast<long>(bytes));
				
				if (loop == false)
				*/
				m_eos = true;
				break;
			}
		}
	}
	return decoded;
}


size_type OggDecoder::decodeInterleaved(SampleType p_type,
                                        void*      p_buffer,
                                        size_type  p_channels,
                                        size_type  p_frames,
                                        size_type  p_offset)
{
	TT_NULL_ASSERT(p_buffer);
	
	s32 decoded = 0;
	
	size_type index = p_offset;
	
	if (p_frames == 0)
	{
		return p_frames;
	}
	
	while (p_frames > 0 && m_eos == false)
	{
		while (p_frames > 0 && m_eos == false)
		{
			int result = 1;
			if (m_samplesReady == 0)
			{
				result = ogg_sync_pageout(&m_syncState, &m_page);
			}
			
			if (result == 0)
			{
				// need more data
				break;
			}
			
			if (result < 0)
			{
				// missing or corrupt data at this page position
				TT_WARN("Corrupt or missing data in bitstream; continuing...\n");
			}
			else
			{
				if (m_samplesReady == 0)
				{
					ogg_stream_pagein(&m_streamState, &m_page); // can safely ignore errors at this point
				}
				for (;;)
				{
					if (m_samplesReady == 0)
					{
						result = ogg_stream_packetout(&m_streamState, &m_packet);
					}
					else
					{
						result = 1;
					}
					
					if (result == 0)
					{
						// need more data
						break;
					}
					
					if (result < 0)
					{
						// missing or corrupt data at this page position
						// no reason to complain; already complained above
					}
					else
					{
						// we have a packet. Decode it
						
						// test for success!
						
						if (m_samplesReady == 0)
						{
							if (vorbis_synthesis(&m_block, &m_packet) == 0)
							{
								vorbis_synthesis_blockin(&m_dspState, &m_block);
							}
						}
						
						// **pcm is a multichannel float vector.  In stereo, for
						// example, pcm[0] is left, and pcm[1] is right.  samples is
						// the size of each channel.  Convert the float values
						// (-1.<=range<=1.) to whatever PCM format and write it out
						
						while (m_samplesReady != 0 || (m_samplesReady = vorbis_synthesis_pcmout(&m_dspState, &m_decodeBuffer) ) > 0)
						{
							bool clipflag = false;
							int decodedSamples = (m_samplesReady < m_framesPerBuffer ? m_samplesReady : m_framesPerBuffer);
							
							for (int j = m_samplesStart; j < decodedSamples; ++j)
							{
								for (int i = 0; i < p_channels; i++)
								{
									float sample;
									if (i < m_info.channels)
									{
										sample = m_decodeBuffer[i][j];
										
										// clamp
										if (sample < -1.0f)
										{
											sample = -1.0f;
											clipflag = true;
										}
										else if (sample > 1.0f)
										{
											sample = 1.0f;
											clipflag = true;
										}
									}
									else
									{
										sample = 0.0f;
									}
									
									size_type idx = (index * p_channels) + i;
									
									// convert to destination type
									switch (p_type)
									{
									case SampleType_Signed8:
										reinterpret_cast<s8*>(p_buffer)[idx] = static_cast<s8>(sample * 127.0f);
										break;
										
									case SampleType_Unsigned8:
										reinterpret_cast<u8*>(p_buffer)[idx] = static_cast<u8>((sample + 1.0f) * 127.5f);
										break;
										
									case SampleType_Signed16:
										reinterpret_cast<s16*>(p_buffer)[idx] = static_cast<s16>(sample * 32767.0f);
										break;
										
									case SampleType_Float:
										reinterpret_cast<float*>(p_buffer)[idx] = sample;
										break;
										
									default:
										break;
									}
								}
								
								++index;
								++m_samplesStart;
								--p_frames;
								++decoded;
								
								if (p_frames == 0)
								{
									return decoded;
								}
							}
							m_samplesStart = 0;
							m_samplesReady = 0;
							
							if (clipflag)
							{
								// just ignore this
								//TT_WARN("Clipping in frame %lld\n", m_dspState.sequence);
							}
							
							// tell libvorbis how many samples we actually consumed
							vorbis_synthesis_read(&m_dspState, decodedSamples);
						}
					}
				}
			}
		}
		
		if (p_frames > 0)
		{
			// read the next part of data
			m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
			
			fs::size_type bytes = m_file->read(m_buffer, BufferSize);
			
			if (ogg_sync_wrote(&m_syncState, static_cast<long>(bytes)) != 0)
			{
				TT_WARN("Overflow in ogg_sync_wrote");
			}
			
			if (bytes == 0)
			{
				m_eos = true;
				break;
			}
		}
	}
	return decoded;
}


bool OggDecoder::rewind()
{
	// Brute-force decoder reset: destroy entire decoding state and re-initialize
	destroy();
	m_file->seekToBegin();
	return init();
	
	/* FIXME: This implementation is more light-weight than simply resetting the entire decoder,
	//        but yields some garbage data at the start when decoding after rewind.
	ogg_stream_reset(&m_streamState);
	ogg_sync_reset(&m_syncState);
	
	m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
	m_file->seekToBegin();
	fs::size_type bytes = m_file->read(m_buffer, BufferSize);
	ogg_sync_wrote(&m_syncState, static_cast<long>(bytes));
	
	m_eos = false;
	return true;
	// */
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool OggDecoder::init()
{
	TT_NULL_ASSERT(m_file);
	
	m_buffer          = 0;
	m_framesPerBuffer = 0;
	m_decodeBuffer    = 0;
	m_eos             = false;
	m_samplesReady    = 0;
	m_samplesStart    = 0;
	m_sampleType      = SampleType_Float;
	m_channelCount    = 0;
	m_sampleRate      = 0;
	m_sampleSize      = 0;
	m_frameCount      = 0;
	
	// clear member variables
	mem::zero8(&m_syncState,   sizeof(ogg_sync_state));
	mem::zero8(&m_streamState, sizeof(ogg_stream_state));
	mem::zero8(&m_page,        sizeof(ogg_page));
	mem::zero8(&m_packet,      sizeof(ogg_packet));
	
	mem::zero8(&m_info,        sizeof(vorbis_info));
	mem::zero8(&m_comment,     sizeof(vorbis_comment));
	mem::zero8(&m_dspState,    sizeof(vorbis_dsp_state));
	mem::zero8(&m_block,       sizeof(vorbis_block));
	
	m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
	
	// grab some data at the head of the stream.  We want the first page
	// (which is guaranteed to be small and only contain the Vorbis
	// stream initial header) We need the first page to get the stream
	// serialno.
	
	// submit a 4k block to libvorbis' Ogg layer
	
	fs::size_type bytes = m_file->read(m_buffer, BufferSize);
	
	ogg_sync_wrote(&m_syncState, static_cast<long>(bytes));
	
	// Get the first page.
	if (ogg_sync_pageout(&m_syncState, &m_page) != 1)
	{
		// have we simply run out of data?  If so, we're done.
		if (bytes < BufferSize)
		{
			return true;
		}
		
		// error case.  Must not be Vorbis data
		TT_PANIC("Input does not appear to be an Ogg bitstream.\n");
		return false;
	}
	
	// Get the serial number and set up the rest of decode.
	// serialno first; use it to set up a logical stream
	ogg_stream_init(&m_streamState, ogg_page_serialno(&m_page));
	
	// extract the initial header from the first page and verify that the
	// Ogg bitstream is in fact Vorbis data
	
	// I handle the initial header first instead of just having the code
	// read all three Vorbis headers at once because reading the initial
	// header is an easy way to identify a Vorbis bitstream and it's
	// useful to see that functionality separated out.
	
	vorbis_info_init(&m_info);
	vorbis_comment_init(&m_comment);
	if (ogg_stream_pagein(&m_streamState, &m_page) < 0)
	{
		// error; stream version mismatch perhaps
		TT_PANIC("Error reading first page of Ogg bitstream data.");
		return false;
	}
	
	if (ogg_stream_packetout(&m_streamState, &m_packet) != 1)
	{
		// no page? must not be vorbis
		TT_PANIC("Error reading initial header packet.");
		return false;
	}
	
	if (vorbis_synthesis_headerin(&m_info, &m_comment, &m_packet) < 0)
	{
		// error case; not a vorbis header
		TT_PANIC("This Ogg bitstream does not contain Vorbis audio data.");
		return false;
	}
	
	// At this point, we're sure we're Vorbis.  We've set up the logical
	// (Ogg) bitstream decoder.  Get the comment and codebook headers and
	// set up the Vorbis decoder
	
	// The next two packets in order are the comment and codebook headers.
	// They're likely large and may span multiple pages.  Thus we read
	// and submit data until we get our two pacakets, watching that no
	// pages are missing.  If a page is missing, error out; losing a
	// header page is the only place where missing data is fatal.
	
	int packets = 0;
	while (packets < 2)
	{
		while (packets < 2)
		{
			int result = ogg_sync_pageout(&m_syncState, &m_page);
			if (result == 0)
			{
				break; // Need more data
			}
			// Don't complain about missing or corrupt data yet.  We'll
			// catch it at the packet output phase
			if (result == 1)
			{
				ogg_stream_pagein(&m_streamState, &m_page); // we can ignore any errors here
				                                            // as they'll also become apparent
				                                            // at packetout
				while (packets < 2)
				{
					result = ogg_stream_packetout(&m_streamState, &m_packet);
					if (result == 0)
					{
						break;
					}
					
					if (result < 0)
					{
						// Uh oh; data at some point was corrupted or missing!
						// We can't tolerate that in a header. Die.
						TT_PANIC("Corrupt secondary header. Exiting.");
						return false;
					}
					vorbis_synthesis_headerin(&m_info, &m_comment, &m_packet);
					packets++;
				}
			}
		}
		// no harm in not checking before adding more
		m_buffer = ogg_sync_buffer(&m_syncState, BufferSize);
		
		bytes = m_file->read(m_buffer, BufferSize);
		
		if (bytes == 0 && packets < 2)
		{
			TT_PANIC("End of file before finding all Vorbis headers!");
			return false;
		}
		ogg_sync_wrote(&m_syncState, static_cast<long>(bytes));
	}
	
	m_framesPerBuffer = BufferSize / m_info.channels;
	m_eos = false;
	
	// OK, got and parsed all three headers. Initialize the Vorbis
	// packet->PCM decoder.
	vorbis_synthesis_init(&m_dspState, &m_info); // central decode state
	vorbis_block_init(&m_dspState, &m_block);    // local state for most of the decode
	                                             // so multiple block decodes can
	                                             // proceed in parallel.  We could init
	                                             // multiple vorbis_block structures
	                                             // for vd here
	
	// we're ready to decode
	
	m_channelCount = static_cast<size_type>(m_info.channels);
	m_sampleRate   = static_cast<size_type>(m_info.rate);
	m_sampleSize   = 32;
	m_frameCount   = 0; // unknown
	
	return true;
}


void OggDecoder::destroy()
{
	// clean up this logical bitstream;
	ogg_stream_clear(&m_streamState);
	
	// ogg_page and ogg_packet structs always point to storage in
	// libvorbis. They're never freed or manipulated directly
	
	vorbis_block_clear(&m_block);
	vorbis_dsp_clear(&m_dspState);
	vorbis_comment_clear(&m_comment);
	vorbis_info_clear(&m_info); // must be called last
	
	// OK, clean up the framer
	ogg_sync_clear(&m_syncState);
}

// Namespace end
}
}
}
}
