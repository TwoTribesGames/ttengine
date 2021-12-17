#ifndef INC_TT_AUDIO_CHIBI_XMENVELOPE_H
#define INC_TT_AUDIO_CHIBI_XMENVELOPE_H

#include <tt/audio/chibi/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace chibi {

struct XMEnvelope
{
	
	enum Flag
	{
		Flag_PointCountMask = (1 << 5) - 1, // First 5 bits indicate point count
		Flag_Enabled        = (1 << 7),     // Envelope is enabled
		Flag_LoopEnabled    = (1 << 6),     // Loop is enabled
		Flag_SustainEnabled = (1 << 5)      // If Loop is enabled, Enable Sustain
	};
	
	u16 points[XMConstant_MaxEnvelopePoints]; // Envelope points
	u8 flags; // Envelope Flags & Point count
	
	u8 sustainIndex;   // Sustain Point Index
	u8 loopBeginIndex; // Loop Begin Point Index
	u8 loopEndIndex;   // Loop End  Point Index
	
	static inline u16 getOffset(u16 p_point)
	{
		return static_cast<u16>(p_point & 0x1FF);
	}
	
	static inline u16 getValue(u16 p_point)
	{
		return static_cast<u16>(p_point >> 9);
	}
};


} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMENVELOPE_H
