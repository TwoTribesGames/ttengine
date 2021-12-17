#if !defined(INC_TT_AUDIO_CODEC_SAMPLECONVERTER_H)
#define INC_TT_AUDIO_CODEC_SAMPLECONVERTER_H

#include <tt/audio/codec/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace codec {

class SampleConverter
{
public:
	virtual ~SampleConverter() {}
	
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame) = 0;
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample) = 0;
};


SampleConverter& getConverter(SampleType p_source, SampleType p_dest);


// U8 conversion

class SampleConverterU8toU8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<u8**>(p_dest)[p_channel][p_destFrame] = 
			reinterpret_cast<u8**>(p_source)[p_channel][p_sourceFrame];
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<u8*>(p_dest)[p_destSample] = 
			reinterpret_cast<u8*>(p_source)[p_sourceSample];
	}
};


class SampleConverterU8toS8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s8**>(p_dest)[p_channel][p_destFrame] = static_cast<s8>(
			reinterpret_cast<u8**>(p_source)[p_channel][p_sourceFrame] - 128);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s8*>(p_dest)[p_destSample] = static_cast<s8>(
			reinterpret_cast<u8*>(p_source)[p_sourceSample] - 128);
	}
};


class SampleConverterU8toS16 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s16**>(p_dest)[p_channel][p_destFrame] = static_cast<s16>((
			reinterpret_cast<u8**>(p_source)[p_channel][p_sourceFrame] - 128) * 256);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s16*>(p_dest)[p_destSample] = static_cast<s16>((
			reinterpret_cast<u8*>(p_source)[p_sourceSample] - 128) * 256);
	}
};


class SampleConverterU8toFloat : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<float**>(p_dest)[p_channel][p_destFrame] = (static_cast<float>(
			reinterpret_cast<u8**>(p_source)[p_channel][p_sourceFrame]) -128.0f) / 256.0f;
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<float*>(p_dest)[p_destSample] = (static_cast<float>(
			reinterpret_cast<u8*>(p_source)[p_sourceSample]) -128.0f) / 256.0f;
	}
};


// S8 conversion

class SampleConverterS8toU8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<u8**>(p_dest)[p_channel][p_destFrame] = static_cast<u8>(
			reinterpret_cast<s8**>(p_source)[p_channel][p_sourceFrame] + 128);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<u8*>(p_dest)[p_destSample] = static_cast<u8>(
			reinterpret_cast<s8*>(p_source)[p_sourceSample] + 128);
	}
};


class SampleConverterS8toS8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s8**>(p_dest)[p_channel][p_destFrame] =
			reinterpret_cast<s8**>(p_source)[p_channel][p_sourceFrame];
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s8*>(p_dest)[p_destSample] =
			reinterpret_cast<s8*>(p_source)[p_sourceSample];
	}
};


class SampleConverterS8toS16 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s16**>(p_dest)[p_channel][p_destFrame] = static_cast<s16>(
			reinterpret_cast<s8**>(p_source)[p_channel][p_sourceFrame] * 256);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s16*>(p_dest)[p_destSample] = static_cast<s16>(
			reinterpret_cast<s8*>(p_source)[p_sourceSample] * 256);
	}
};


class SampleConverterS8toFloat : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<float**>(p_dest)[p_channel][p_destFrame] = static_cast<float>(
			reinterpret_cast<s8**>(p_source)[p_channel][p_sourceFrame]) / 128.0f;
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<float*>(p_dest)[p_destSample] = static_cast<float>(
			reinterpret_cast<s8*>(p_source)[p_sourceSample]) / 128.0f;
	}
};


// S16 conversion

class SampleConverterS16toU8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<u8**>(p_dest)[p_channel][p_destFrame] = static_cast<u8>((
			reinterpret_cast<s16**>(p_source)[p_channel][p_sourceFrame] / 256) + 128);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<u8*>(p_dest)[p_destSample] = static_cast<u8>((
			reinterpret_cast<s16*>(p_source)[p_sourceSample] / 256) + 128);
	}
};


class SampleConverterS16toS8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s8**>(p_dest)[p_channel][p_destFrame] = static_cast<s8>(
			reinterpret_cast<s16**>(p_source)[p_channel][p_sourceFrame] / 256);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s8*>(p_dest)[p_destSample] = static_cast<s8>(
			reinterpret_cast<s16*>(p_source)[p_sourceSample] / 256);
	}
};


class SampleConverterS16toS16 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s16**>(p_dest)[p_channel][p_destFrame] =
			reinterpret_cast<s16**>(p_source)[p_channel][p_sourceFrame];
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s16*>(p_dest)[p_destSample] =
			reinterpret_cast<s16*>(p_source)[p_sourceSample];
	}
};


class SampleConverterS16toFloat : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<float**>(p_dest)[p_channel][p_destFrame] = static_cast<float>(
			reinterpret_cast<s16**>(p_source)[p_channel][p_sourceFrame]) / 32767.0f;
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<float*>(p_dest)[p_destSample] = static_cast<float>(
			reinterpret_cast<s16*>(p_source)[p_sourceSample]) / 32767.0f;
	}
};


// Float conversion

class SampleConverterFloattoU8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<u8**>(p_dest)[p_channel][p_destFrame] = static_cast<u8>(
			(reinterpret_cast<float**>(p_source)[p_channel][p_sourceFrame] + 1.0f) * 127.5f);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<u8*>(p_dest)[p_destSample] = static_cast<u8>(
			(reinterpret_cast<float*>(p_source)[p_sourceSample] + 1.0f) * 127.5f);
	}
};


class SampleConverterFloattoS8 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s8**>(p_dest)[p_channel][p_destFrame] = static_cast<s8>(
			reinterpret_cast<float**>(p_source)[p_channel][p_sourceFrame] * 127.0f);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s8*>(p_dest)[p_destSample] = static_cast<s8>(
			reinterpret_cast<float*>(p_source)[p_sourceSample] * 127.0f);
	}
};


class SampleConverterFloattoS16 : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<s16**>(p_dest)[p_channel][p_destFrame] = static_cast<s16>(
			reinterpret_cast<float**>(p_source)[p_channel][p_sourceFrame] * 32767.0f);
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<s16*>(p_dest)[p_destSample] = static_cast<s16>(
			reinterpret_cast<float*>(p_source)[p_sourceSample] * 32767.0f);
	}
};


class SampleConverterFloattoFloat : public SampleConverter
{
public:
	virtual void convert(void**    p_source,
	                     void**    p_dest,
	                     size_type p_channel,
	                     size_type p_sourceFrame,
	                     size_type p_destFrame)
	{
		reinterpret_cast<float**>(p_dest)[p_channel][p_destFrame] =
			reinterpret_cast<float**>(p_source)[p_channel][p_sourceFrame];
	}
	
	virtual void convert(void*     p_source,
	                     void*     p_dest,
	                     size_type p_sourceSample,
	                     size_type p_destSample)
	{
		reinterpret_cast<float*>(p_dest)[p_destSample] =
			reinterpret_cast<float*>(p_source)[p_sourceSample];
	}
};

// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_SAMPLECONVERTER_H)
