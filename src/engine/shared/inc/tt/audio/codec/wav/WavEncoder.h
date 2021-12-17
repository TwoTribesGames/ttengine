#if !defined(INC_TT_AUDIO_CODEC_WAV_WAVENCODER_H)
#define INC_TT_AUDIO_CODEC_WAV_WAVENCODER_H

#include <string>

#include <tt/audio/codec/Encoder.h>
#include <tt/fs/types.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

class WavEncoder : public Encoder
{
public:
	/*! \brief Instantiates and initializes encoder object.
	    \param p_type Sample type to encode to.
	    \param p_channelCount Number of channels to encode to.
	    \param p_sampleRate Sample rate to encode to.
	    \param p_filename File to encode to.
	    \param p_fs Filesystem to use.*/
	WavEncoder(SampleType         p_type,
	           size_type          p_channelCount,
	           size_type          p_sampleRate,
	           const std::string& p_filename,
	           fs::identifier     p_fs = 0);
	
	/*! \brief Finalizes encoded file.*/
	virtual ~WavEncoder();
	
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
	
	/*! \brief Encodes from noninterleaved buffer.
	    \param p_buffer array of pointers to getChannelCount() number of buffers
	                    of p_frames samples in getSampleType() format.
	    \param p_frames number of samples per buffer.
	    \param p_offset number of frames to skip in buffers.
	    \return The number of frames encoded.*/
	virtual size_type encode(const void** p_buffer,
	                         size_type    p_frames,
	                         size_type    p_offset = 0);
	
	/*! \brief Encodes from interleaved buffer.
	    \param p_buffer Buffer of getChannelCount() * p_frames samples in getSampleType() format.
	    \param p_frames number of frames to decode.
	    \param p_offset number of frames to skip in buffer.
	    \return The number of frames decoded.*/
	virtual size_type encodeInterleaved(const void* p_buffer,
	                                    size_type   p_frames,
	                                    size_type   p_offset = 0);
	
private:
	WavEncoder(const WavEncoder&);
	const WavEncoder& operator=(const WavEncoder&);
	
	fs::FilePtr m_file;
	
	SampleType m_sampleType;
	
	size_type m_channelCount;
	size_type m_sampleRate;
	size_type m_sampleSize;
	size_type m_frameCount;
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_WAV_WAVENCODER_H)
