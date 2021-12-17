#if !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERIMAADPCM_H)
#define INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERIMAADPCM_H

#include <vector>

#include <tt/audio/codec/ttadpcm/ImaAdpcm.h>
#include <tt/audio/codec/wav/WavFormatDecoder.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

class WavFormatDecoderImaAdpcm : public WavFormatDecoder
{
public:
	WavFormatDecoderImaAdpcm(WavFormat& p_format, const fs::FilePtr& p_file);
	virtual ~WavFormatDecoderImaAdpcm();
	
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
	WavFormatDecoderImaAdpcm(const WavFormatDecoderImaAdpcm&);
	const WavFormatDecoderImaAdpcm& operator=(const WavFormatDecoderImaAdpcm&);
	
	typedef std::vector<ttadpcm::ADPCMState> DecodeStates;
	
	size_type m_sampleCount;
	size_type m_frameCount;
	size_type m_samplesPerBlock;
	size_type m_framesPerBlock;
	
	size_type m_blockCount;
	size_type m_position;
	
	DecodeStates m_decodeStates;
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODERIMAADPCM_H)
