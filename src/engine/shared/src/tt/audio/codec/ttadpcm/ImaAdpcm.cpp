#include <tt/audio/codec/ttadpcm/ImaAdpcm.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace codec {
namespace ttadpcm {

// Index table taken from http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
static const int gs_indexTable[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};


// Step table taken from http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
// "Note that many programs use slight deviations from the following table,
//  but such deviations are negligible"
static const int gs_stepTable[89] =
{
	    7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
	   19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	   50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	  130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	  876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	 2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	 5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


/*
    IMA ADPCM decode function
    Based on information found on http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
*/
s16 decode(ADPCMState& p_state, u8 p_nibble)
{
	int stepSize = gs_stepTable[p_state.stepIndex];
	
	// delta = (p_nibble + 0.5) * stepSize / 4
	// may be optimized to 
	// delta = ((stepSize * p_nibble) + (stepSize / 2)) / 4
	// may be optimized to
	// delta = (stepSize * p_nibble / 4) + (stepSize / 8)
	// please note that nibble is a sign/magnitude pair and should not be seen as a signed integer.
	
	// optimization to be used when hardware multiplication is either heavy or not supported
	int delta;
	
	switch (p_nibble)
	{
	case 0:
		delta = stepSize >> 3;
		break;
		
	case 1:
		delta = (stepSize >> 3) + (stepSize >> 2);
		break;
		
	case 2:
		delta = (stepSize >> 3) + (stepSize >> 1);
		break;
		
	case 3:
		delta = (stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1);
		break;
		
	case 4:
		delta = (stepSize >> 3) + stepSize;
		break;
		
	case 5:
		delta = (stepSize >> 3) + (stepSize >> 2) + stepSize;
		break;
		
	case 6:
		delta = (stepSize >> 3) + (stepSize >> 1) + stepSize;
		break;
		
	case 7:
		delta = (stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1) + stepSize;
		break;
		
	case 8:
		delta = -(stepSize >> 3);
		break;
		
	case 9:
		delta = -((stepSize >> 3) + (stepSize >> 2));
		break;
		
	case 10:
		delta = -((stepSize >> 3) + (stepSize >> 1));
		break;
		
	case 11:
		delta = -((stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1));
		break;
		
	case 12:
		delta = -((stepSize >> 3) + stepSize);
		break;
		
	case 13:
		delta = -((stepSize >> 3) + (stepSize >> 2) + stepSize);
		break;
		
	case 14:
		delta = -((stepSize >> 3) + (stepSize >> 1) + stepSize);
		break;
		
	case 15:
		delta = -((stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1) + stepSize);
		break;
		
	default:
		delta = 0;
		break;
	}
	
	int sample = p_state.predictor + delta;
	
	// saturate
	if (sample < -32768)
	{
		sample = -32768;
	}
	else if (sample > 32767)
	{
		sample = 32767;
	}
	p_state.predictor = sample;
	
	int stepIndex = p_state.stepIndex + gs_indexTable[p_nibble];
	
	// saturate
	if (stepIndex < 0)
	{
		stepIndex = 0;
	}
	else if (stepIndex > 88)
	{
		stepIndex = 88;
	}
	p_state.stepIndex = stepIndex;
	
	return static_cast<s16>(sample);
}


/*
    IMA ADPCM encode function
    Based on the "new WAVE Types" document by Microsoft
    see http://www.wotsit.org/download.asp?f=wavecomp&sc=288253224
*/
u8 encode(ADPCMState& p_state, s16 p_sample)
{
	int diff = p_sample - p_state.predictor;
	int nibble = 0;
	
	if (diff < 0)
	{
		nibble = 0x08;
		diff = -diff;
	}
	else
	{
		nibble = 0;
	}
	
	if (diff >= gs_stepTable[p_state.stepIndex])
	{
		nibble |= 0x04;
		diff = diff - gs_stepTable[p_state.stepIndex];
	}
	
	if (diff >= (gs_stepTable[p_state.stepIndex] >> 1))
	{
		nibble |= 0x02;
		diff = diff - (gs_stepTable[p_state.stepIndex] >> 1);
	}
	
	if (diff >= (gs_stepTable[p_state.stepIndex] >> 2))
	{
		nibble |= 0x01;
		diff = diff - (gs_stepTable[p_state.stepIndex] >> 2);
	}
	
	int stepSize = gs_stepTable[p_state.stepIndex];
	int delta;
	
	switch (nibble)
	{
	case 0:
		delta = stepSize >> 3;
		break;
		
	case 1:
		delta = (stepSize >> 3) + (stepSize >> 2);
		break;
		
	case 2:
		delta = (stepSize >> 3) + (stepSize >> 1);
		break;
		
	case 3:
		delta = (stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1);
		break;
		
	case 4:
		delta = (stepSize >> 3) + stepSize;
		break;
		
	case 5:
		delta = (stepSize >> 3) + (stepSize >> 2) + stepSize;
		break;
		
	case 6:
		delta = (stepSize >> 3) + (stepSize >> 1) + stepSize;
		break;
		
	case 7:
		delta = (stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1) + stepSize;
		break;
		
	case 8:
		delta = -(stepSize >> 3);
		break;
		
	case 9:
		delta = -((stepSize >> 3) + (stepSize >> 2));
		break;
		
	case 10:
		delta = -((stepSize >> 3) + (stepSize >> 1));
		break;
		
	case 11:
		delta = -((stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1));
		break;
		
	case 12:
		delta = -((stepSize >> 3) + stepSize);
		break;
		
	case 13:
		delta = -((stepSize >> 3) + (stepSize >> 2) + stepSize);
		break;
		
	case 14:
		delta = -((stepSize >> 3) + (stepSize >> 1) + stepSize);
		break;
		
	case 15:
		delta = -((stepSize >> 3) + (stepSize >> 2) + (stepSize >> 1) + stepSize);
		break;
		
	default:
		delta = 0;
		break;
	}
	
	int predictor = p_state.predictor + delta;
	
	if (predictor > 32767)
	{
		predictor = 32767;
	}
	if (predictor < -32768)
	{
		predictor = -32768;
	}
	
	int index = p_state.stepIndex + gs_indexTable[nibble];
	if (index > 88)
	{
		index = 88;
	}
	if (index < 0)
	{
		index = 0;
	}
	p_state.predictor = predictor;
	p_state.stepIndex = index;
	return static_cast<u8>(nibble);
}

// namespace end
}
}
}
}
