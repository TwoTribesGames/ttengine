#include <tt/audio/codec/SampleConverter.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace codec {

static SampleConverterU8toU8     s_u8Tou8;
static SampleConverterU8toS8     s_u8Tos8;
static SampleConverterU8toS16    s_u8Tos16;
static SampleConverterU8toFloat  s_u8Tofloat;

static SampleConverterS8toU8     s_s8Tou8;
static SampleConverterS8toS8     s_s8Tos8;
static SampleConverterS8toS16    s_s8Tos16;
static SampleConverterS8toFloat  s_s8Tofloat;

static SampleConverterS16toU8    s_s16Tou8;
static SampleConverterS16toS8    s_s16Tos8;
static SampleConverterS16toS16   s_s16Tos16;
static SampleConverterS16toFloat s_s16Tofloat;

static SampleConverterFloattoU8    s_floatTou8;
static SampleConverterFloattoS8    s_floatTos8;
static SampleConverterFloattoS16   s_floatTos16;
static SampleConverterFloattoFloat s_floatTofloat;

SampleConverter& getConverter(SampleType p_source, SampleType p_dest)
{
	switch (p_source)
	{
	case SampleType_Unsigned8:
		switch (p_dest)
		{
		case SampleType_Unsigned8: return s_u8Tou8;
		case SampleType_Signed8:   return s_u8Tos8;
		case SampleType_Signed16:  return s_u8Tos16;
		case SampleType_Float:     return s_u8Tofloat;
		default: break;
		}
		break;
		
	case SampleType_Signed8:
		switch (p_dest)
		{
		case SampleType_Unsigned8: return s_s8Tou8;
		case SampleType_Signed8:   return s_s8Tos8;
		case SampleType_Signed16:  return s_s8Tos16;
		case SampleType_Float:     return s_s8Tofloat;
		default: break;
		}
		break;
		
	case SampleType_Signed16:
		switch (p_dest)
		{
		case SampleType_Unsigned8: return s_s16Tou8;
		case SampleType_Signed8:   return s_s16Tos8;
		case SampleType_Signed16:  return s_s16Tos16;
		case SampleType_Float:     return s_s16Tofloat;
		default: break;
		}
		break;
		
	case SampleType_Float:
		switch (p_dest)
		{
		case SampleType_Unsigned8: return s_floatTou8;
		case SampleType_Signed8:   return s_floatTos8;
		case SampleType_Signed16:  return s_floatTos16;
		case SampleType_Float:     return s_floatTofloat;
		default: break;
		}
		break;
		
	default: break;
	}
	
	TT_PANIC("Sample type source (%d) or destination (%d) is invalid.", p_source, p_dest);
	
	// return a invalid reference
	return s_u8Tou8;
}


// namespace end
}
}
}
