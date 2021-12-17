#ifndef INC_TT_AUDIO_CHIBI_XMINSTRUMENT_H
#define INC_TT_AUDIO_CHIBI_XMINSTRUMENT_H

#include <cstddef>

#include <tt/audio/chibi/types.h>
#include <tt/audio/chibi/XMEnvelope.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace chibi {

struct XMInstrument
{
	// Envelopes
	XMEnvelope volumeEnvelope;
	XMEnvelope panEnvelope;
	
	// Instrument Vibrato
	XMVibratoType vibratoType;
	
	u8 vibratoSweep;
	u8 vibratoDepth;
	u8 vibratoRate;
	u16 fadeout;
	
	// Note/Sample table
	u8 noteSample[XMConstant_MaxNotes / 2]; // sample for each note, in nibbles
	
	// Sample Data
	XMSample* samples;
	u8        sampleCount;
	
	
	// No direct instantiation or copying
	XMInstrument();
	~XMInstrument();
	XMInstrument(const XMInstrument&);
	XMInstrument& operator=(const XMInstrument&);
	
	static void* operator new(std::size_t p_size);
	static void* operator new[](std::size_t p_size);
	static void  operator delete(void* p_block);
	static void  operator delete[](void* p_block);
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMINSTRUMENT_H
