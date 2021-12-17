#if !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERPCM_H)
#define INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERPCM_H

#include <tt/audio/codec/wav/WavFormatDecoder.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

class WavFormatDecoderPcm : public WavFormatDecoder
{
public:
	WavFormatDecoderPcm(WavFormat& p_format, const fs::FilePtr& p_file);
	virtual ~WavFormatDecoderPcm();
	
	virtual void parseFact(size_type p_chunkSize);
	virtual void parseExFormat(size_type p_chunkSize);
	virtual void parseData(size_type p_chunkSize);
	
	virtual size_type  getSampleCount() const;
	virtual size_type  getFrameCount() const;
	virtual size_type  getSamplesPerBlock() const;
	virtual size_type  getFramesPerBlock() const;
	virtual SampleType getSampleType() const;
	virtual size_type  getBlockBufferSize() const;
	
	virtual size_type decodeBlocks(void** p_buffer, size_type p_blocks);
	virtual size_type decodeBlocksInterleaved(void* p_buffer, size_type p_blocks);
	
	virtual bool rewind();
	
private:
	WavFormatDecoderPcm(const WavFormatDecoderPcm&);
	const WavFormatDecoderPcm& operator=(const WavFormatDecoderPcm&);
	
	size_type m_sampleCount;
	size_type m_frameCount;
	size_type m_samplesPerBlock;
	size_type m_framesPerBlock;
	
	size_type m_position;
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERPCM_H)
