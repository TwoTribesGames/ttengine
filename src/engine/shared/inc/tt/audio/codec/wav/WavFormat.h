#if !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMAT_H)
#define INC_TT_AUDIO_CODEC_WAV_WAVFORMAT_H

#include <tt/fs/types.h>


namespace tt {
namespace audio {
namespace codec {
namespace wav {

struct WavFormat
{
public:
	size_type format;
	size_type channelCount;
	size_type samplesPerSec;
	size_type avgBytesPerSec;
	size_type blockSize;
	size_type sampleSize;
	size_type extraSize;
	
	
	inline WavFormat()
	:
	format(0),
	channelCount(0),
	samplesPerSec(0),
	avgBytesPerSec(0),
	blockSize(0),
	sampleSize(0),
	extraSize(0)
	{ }
};

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_WAV_WAVFORMAT_H)
