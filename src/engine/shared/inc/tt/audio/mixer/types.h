#if !defined(INC_TT_AUDIO_MIXER_TYPES_H)
#define INC_TT_AUDIO_MIXER_TYPES_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace mixer {

// Typedefs

typedef int size_type;           //!< Sample size / position type
typedef int channel_type;        //!< Channel index type
typedef int freq_type;           //!< Frequency type
typedef int pan_type;            //!< Panning type
typedef int vol_type;            //!< Volume type
typedef int duty_type;           //!< Duty ratio type
typedef void (*callback)(void*); //!< Callback function used by mixer


// External values
extern const vol_type c_minVol; //!< Minimum volume
extern const vol_type c_maxVol; //!< Maximum volume

extern const pan_type c_minPan;    //!< Minimum panning
extern const pan_type c_maxPan;    //!< Maximum panning
extern const pan_type c_leftPan;   //!< Full left panning
extern const pan_type c_centerPan; //!< Centered panning
extern const pan_type c_rightPan;  //!< Full right panning

extern const duty_type c_minDuty; //!< Minimum duty
extern const duty_type c_maxDuty; //!< Maximum duty
extern const duty_type c_1_8Duty; //!< 1/8th duty (12.5%)
extern const duty_type c_2_8Duty; //!< 2/8th duty (25.0%)
extern const duty_type c_3_8Duty; //!< 3/8th duty (37.5%)
extern const duty_type c_4_8Duty; //!< 4/8th duty (50.0%)
extern const duty_type c_5_8Duty; //!< 5/8th duty (62.5%)
extern const duty_type c_6_8Duty; //!< 6/8th duty (75.0%)
extern const duty_type c_7_8Duty; //!< 7/8th duty (87.5%)


//Enums

enum LoopType
{
	LoopType_None,     //!< No looping
	LoopType_Loop,     //!< Simple looping
	LoopType_PingPong  //!< Ping pong looping
};

enum SampleType
{
	SampleType_PCM8, //!< Signed 8 bit PCM
	SampleType_PCM16 //!< Signed 16 bit PCM
};

enum NoiseType
{
	NoiseType_White,        //!< Uniform white noise
	NoiseType_GausianWhite, //!< Gausian white noise
	NoiseType_Pink,         //!< Pink (1/f) noise
	NoiseType_Brown,        //!< Brown noise
	NoiseType_Blue,         //!< Blue noise
	NoiseType_Purple,       //!< Purple noise
	NoiseType_Grey          //!< Grey noise
};

// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_MIXER_TYPES_H)
