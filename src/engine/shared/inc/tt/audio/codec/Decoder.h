#if !defined(INC_TT_AUDIO_CODEC_DECODER_H)
#define INC_TT_AUDIO_CODEC_DECODER_H

#include <tt/platform/tt_types.h>
#include <tt/audio/codec/types.h>


namespace tt {
namespace audio {
namespace codec {

class Decoder
{
public:
	Decoder() {}
	virtual ~Decoder() {}
	
	/*! \brief Retrieves the number of channels.
	    \return The number of channels.*/
	virtual size_type getChannelCount() const = 0;
	
	/*! \brief Retrieves the sample rate in samples per second (Hz).
	    \return The sample rate.*/
	virtual size_type getSampleRate() const = 0;
	
	/*! \brief Retrieves the sample size in bits.
	    \return The sample size.*/
	virtual size_type getSampleSize() const = 0;
	
	/*! \brief Retrieves the number of frames.
	    \return The number of frames.*/
	virtual size_type getFrameCount() const = 0;
	
	/*! \brief Retrieves the type of samples.
	    \return The sample type.*/
	virtual SampleType getSampleType() const = 0;
	
	/*! \brief Decodes noninterleaved.
	    \param p_type Type of sample to decode to.
	    \param p_buffer array of pointers to p_channels number of buffers of p_frames samples.
	    \param p_channels number of buffers.
	    \param p_frames number of samples per buffer.
	    \param p_offset number of frames to skip in buffers.
	    \return The number of frames decoded.*/
	virtual size_type decode(SampleType p_type,
	                         void**     p_buffer,
	                         size_type  p_channels,
	                         size_type  p_frames,
	                         size_type  p_offset = 0) = 0;
	
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
	                                    size_type  p_offset = 0) = 0;
	
	/*! \brief Rewinds the decoder to the beginning of the file.
	           valid only when the end of the file has been reached.
	    \return Whether rewinding succeeded.*/
	virtual bool rewind() = 0;
	
private:
	Decoder(const Decoder&);
	const Decoder& operator=(const Decoder&);
	
};

typedef tt_ptr<Decoder>::shared DecoderPtr;


// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_DECODER_H)
