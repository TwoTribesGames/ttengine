#if !defined(INC_TT_AUDIO_CODEC_CAF_CAFDECODER_H)
#define INC_TT_AUDIO_CODEC_CAF_CAFDECODER_H

#include <string>
#include <vector>

#include <tt/audio/codec/Decoder.h>
#include <tt/fs/types.h>


namespace tt {
namespace audio {
namespace codec {
namespace caf {

class CafDecoder : public Decoder
{
public:
	explicit CafDecoder(const std::string& p_filename, fs::identifier p_fs = 0);
	virtual ~CafDecoder();
	
	/*! \brief Retrieves the number of channels.
	    \return The number of channels.*/
	virtual size_type getChannelCount() const;
	
	/*! \brief Retrieves the sample rate in samples per second (Hz).
	    \return The sample rate.*/
	virtual size_type getSampleRate() const;
	
	/*! \brief Retrieves the sample size in bits.
	    \return The sample size.*/
	virtual size_type getSampleSize() const;
	
	/*! \brief Retrieves the number of frames.
	    \return The number of frames.*/
	virtual size_type getFrameCount() const;
	
	/*! \brief Retrieves the type of samples.
	    \return The sample type.*/
	virtual SampleType getSampleType() const;
	
	/*! \brief Decodes noninterleaved.
	    \param p_type Type of sample to decode to.
	    \param p_buffer array of pointers to p_channels number of buffers of p_frames samples.
	    \param p_channels number of buffers.
	    \param p_frames number of frames to decode.
	    \param p_offset number of frames to skip in buffers.
	    \return The number of frames decoded.*/
	virtual size_type decode(SampleType p_type,
	                         void**     p_buffer,
	                         size_type  p_channels,
	                         size_type  p_frames,
	                         size_type  p_offset = 0);
	
	/*! \brief Decodes interleaved.
	    \param p_type Type of sample to decode to.
	    \param p_buffer Buffer of p_channels * p_frames samples.
	    \param p_channels number of channels to decode.
	    \param p_frames number of frames to decode.
	    \param p_offset number of frames to skip in buffer.
	    \return The number of frames decoded.*/
	virtual size_type decodeInterleaved(SampleType p_type,
	                                    void*      p_buffer,
	                                    size_type  p_channels,
	                                    size_type  p_frames,
	                                    size_type  p_offset = 0);
	
	/*! \brief Rewinds the decoder to the beginning of the file.
	           valid only when the end of the file has been reached.
	    \return Whether rewinding succeeded.*/
	virtual bool rewind();
	
private:
	CafDecoder(const CafDecoder&);
	const CafDecoder& operator=(const CafDecoder&);
	
	fs::FilePtr m_file;
	
	fs::pos_type m_rewindPosition;
	size_type    m_position;
	
	SampleType m_sampleType;
	
	size_type m_channelCount;
	size_type m_sampleRate;
	size_type m_sampleSize;
	size_type m_frameCount;
	
	bool m_littleEndian;
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_CAF_CAFDECODER_H)
