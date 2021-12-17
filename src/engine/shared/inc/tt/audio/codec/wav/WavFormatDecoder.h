#if !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODER_H)
#define INC_TT_AUDIO_CODEC_WAV_WAVFORMATDECODER_H

#include <tt/audio/codec/types.h>
#include <tt/audio/codec/wav/WavFormat.h>
#include <tt/fs/types.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

class WavFormatDecoder
{
public:
	inline WavFormatDecoder(WavFormat& p_format, const fs::FilePtr& p_file)
	:
	m_file(p_file),
	m_rewindPosition(0),
	m_format(p_format)
	{
	}
	virtual ~WavFormatDecoder() { }
	
	virtual void parseFact(size_type p_chunkSize) = 0;
	virtual void parseExFormat(size_type p_chunkSize) = 0;
	virtual void parseData(size_type p_chunkSize) = 0;
	
	virtual size_type  getSampleCount() const = 0;
	virtual size_type  getFrameCount() const = 0;
	virtual size_type  getSamplesPerBlock() const = 0;
	virtual size_type  getFramesPerBlock() const = 0;
	virtual SampleType getSampleType() const = 0;
	virtual size_type  getBlockBufferSize() const = 0;
	
	virtual size_type decodeBlocks(void** p_buffer, size_type p_blocks) = 0;
	virtual size_type decodeBlocksInterleaved(void* p_buffer, size_type p_blocks) = 0;
	
	virtual bool rewind() = 0;
	
protected:
	fs::FilePtr  m_file;
	fs::pos_type m_rewindPosition;
	WavFormat&   m_format;
	
private:
	WavFormatDecoder(const WavFormatDecoder&);
	const WavFormatDecoder& operator=(const WavFormatDecoder&);
	
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_WAV_WAVDECODER_H)
