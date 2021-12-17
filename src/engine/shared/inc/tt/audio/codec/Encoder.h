#if !defined(INC_TT_AUDIO_CODEC_ENCODER_H)
#define INC_TT_AUDIO_CODEC_ENCODER_H

#include <tt/audio/codec/types.h>


namespace tt {
namespace audio {
namespace codec {

class Encoder
{
public:
	Encoder() {}
	virtual ~Encoder() {}
	
	/*! \brief Retrieves the number of channels.
	    \return The number of channels.*/
	virtual size_type getChannelCount() const = 0;
	
	/*! \brief Retrieves the sample rate in samples per second (Hz).
	    \return The sample rate.*/
	virtual size_type getSampleRate() const = 0;
	
	/*! \brief Retrieves the sample size in bits.
	    \return The sample size.*/
	virtual size_type getSampleSize() const = 0;
	
	/*! \brief Retrieves the current number of frames.
	    \return The current number of frames.*/
	virtual size_type getFrameCount() const = 0;
	
	/*! \brief Retrieves the type of samples.
	    \return The sample type.*/
	virtual SampleType getSampleType() const = 0;
	
	/*! \brief Encodes from noninterleaved buffer.
	    \param p_buffer array of pointers to getChannelCount() number of buffers
	                    of p_frames samples in getSampleType() format.
	    \param p_frames number of samples per buffer.
	    \param p_offset number of frames to skip in buffers.
	    \return The number of frames encoded.*/
	virtual size_type encode(const void** p_buffer,
	                         size_type    p_frames,
	                         size_type    p_offset = 0) = 0;
	
	/*! \brief Encodes from interleaved buffer.
	    \param p_buffer Buffer of getChannelCount() * p_frames samples in getSampleType() format.
	    \param p_frames number of frames to decode.
	    \param p_offset number of frames to skip in buffer.
	    \return The number of frames decoded.*/
	virtual size_type encodeInterleaved(const void* p_buffer,
	                                    size_type   p_frames,
	                                    size_type   p_offset = 0) = 0;

	
private:
	Encoder(const Encoder&);
	const Encoder& operator=(const Encoder&);
	
};

// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_ENCODER_H)
