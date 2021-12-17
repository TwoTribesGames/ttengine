#ifndef INC_TT_AUDIO_CHIBI_TYPES_H
#define INC_TT_AUDIO_CHIBI_TYPES_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace audio {
namespace chibi {


/** MIXER **/

enum XMConstant
{
	XMConstant_MaxNotes                = 96,
	XMConstant_MaxPatterns             = 255,
	XMConstant_MaxInstruments          = 255,
	XMConstant_MaxEnvelopePoints       = 24,
	XMConstant_MaxSamplesPerInstrument = 16,
	XMConstant_MaxOrders               = 256,
	XMConstant_InvalidSampleID         = -1
	
};

/** MIXER **/

enum XMSampleFormat
{
	XMSampleFormat_PCM8,      // signed 8-bits
	XMSampleFormat_PCM16,     // signed 16-bits
	XMSampleFormat_IMA_ADPCM, // ima-adpcm
	XMSampleFormat_Custom     // Custom format, XM_Mixer should support this
};


enum XMLoopType
{
	XMLoopType_Disabled,
	XMLoopType_Forward,
	XMLoopType_PingPong
};


struct XMSampleData
{
	void*          data;      // unused if null
	XMSampleFormat format;
	XMLoopType     loopType;
	u32            loopBegin; // position in audio frames
	u32            loopEnd;   // position in audio frames
	u32            length;    // size in audio frames
};

typedef s32 XMSampleID;


/** INSTRUMENT **/

struct XMSample
{
	XMSampleID sampleId;
	
	s8 baseNote;
	s8 finetune;
	u8 volume;
	u8 pan;
};


enum XMVibratoType
{
	XMVibratoType_Sine,
	XMVibratoType_Square,
	XMVibratoType_SawUp,
	XMVibratoType_SawDown
};


enum XM_PlayerConstants
{
	XM_NoteOff            = 97,
	XM_FieldEmpty         = 0xFF,
	XM_CompNoteBit        = 1,
	XM_CompInstrumentBit  = 2,
	XM_CompVolumeBit      = 4,
	XM_CompCommandBit     = 8,
	XM_CompParameterBit   = 16,
	XM_MaxChannels        = 32
};

enum XM_PatternCompressionCommand
{
	XM_CompSetChannel        = 0,
	XM_CompUseCaches         = 1,
	XM_CompReadFields        = 2,
	XM_CompEndOfPattern      = 3,
	XM_CompReadChannelAdvRow = 4,
//	XM_CompAdvChanUseCache   = 5,
//	XM_CompAdvChanReadFields = 6,
	XM_CompAdvanceRows       = 7
};


struct XMNote
{
	XMNote() :
	note(XM_FieldEmpty),
	instrument(XM_FieldEmpty),
	volume(0),
	command(XM_FieldEmpty),
	parameter(0)
	{
		
	}
	
	u8 note;
	u8 instrument;
	u8 volume;
	u8 command;
	u8 parameter;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_TYPES_H
