#if !defined(INC_TT_AUDIO_CODEC_TYPES_H)
#define INC_TT_AUDIO_CODEC_TYPES_H


namespace tt {
namespace audio {
namespace codec {

// type defines
typedef int size_type;


// enums
enum SampleType
{
	SampleType_Signed8,
	SampleType_Unsigned8,
	SampleType_Signed16,
	SampleType_Float,
	
	SampleType_Unknown
};


// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_TYPES_H)
