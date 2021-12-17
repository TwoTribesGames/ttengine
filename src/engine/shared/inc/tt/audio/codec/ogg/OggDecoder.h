#if !defined(INC_TT_AUDIO_CODEC_OGG_OGGDECODER_H)
#define INC_TT_AUDIO_CODEC_OGG_OGGDECODER_H


#include <string>

#include <tt/audio/codec/Decoder.h>
#include <tt/fs/types.h>

#include <vorbis/codec.h>


namespace tt {
namespace audio {
namespace codec {
namespace ogg {

class OggDecoder : public Decoder
{
public:
	explicit OggDecoder(const std::string& p_filename, fs::identifier p_fs = 0);
	virtual ~OggDecoder();
	
	virtual size_type  getChannelCount() const;
	virtual size_type  getSampleRate()   const;
	virtual size_type  getSampleSize()   const;
	virtual size_type  getFrameCount()   const;
	virtual SampleType getSampleType()   const;
	
	virtual size_type decode(SampleType p_type,
	                         void**     p_buffer,
	                         size_type  p_channels,
	                         size_type  p_frames,
	                         size_type  p_offset = 0);
	virtual size_type decodeInterleaved(SampleType p_type,
	                                    void*      p_buffer,
	                                    size_type  p_channels,
	                                    size_type  p_frames,
	                                    size_type  p_offset = 0);
	
	virtual bool rewind();
	
private:
	bool init();
	void destroy();
	
	// No copying
	OggDecoder(const OggDecoder&);
	const OggDecoder& operator=(const OggDecoder&);
	
	
	fs::FilePtr m_file;
	
	ogg_sync_state   m_syncState;
	ogg_stream_state m_streamState;
	ogg_page         m_page;
	ogg_packet       m_packet;
	
	vorbis_info      m_info;
	vorbis_comment   m_comment;
	vorbis_dsp_state m_dspState;
	vorbis_block     m_block;
	
	char*            m_buffer;
	size_type        m_framesPerBuffer;
	float**          m_decodeBuffer;
	bool             m_eos;
	size_type        m_samplesReady;
	size_type        m_samplesStart;
	
	SampleType m_sampleType;
	
	size_type m_channelCount;
	size_type m_sampleRate;
	size_type m_sampleSize;
	size_type m_frameCount;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TT_AUDIO_CODEC_OGG_OGGDECODER_H)
