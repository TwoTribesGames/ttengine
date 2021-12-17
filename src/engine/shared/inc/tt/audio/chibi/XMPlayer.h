#ifndef INC_AUDIO_CHIBI_XMPLAYER_H
#define INC_AUDIO_CHIBI_XMPLAYER_H

#include <cstddef>

#include <tt/audio/chibi/types.h>


namespace tt {
namespace audio {
namespace chibi {

// forward declarations
class XMMixer;
struct XMInstrument;
struct XMSong;
struct XMEnvelope;

class XMPlayer
{
public:
	XMPlayer();
	
	static void* operator new(std::size_t p_size);
	static void* operator new[](std::size_t p_size);
	static void  operator delete(void* p_block);
	static void  operator delete[](void* p_block);
	
	void            setMixer(XMMixer* p_mixer);
	inline XMMixer* getMixer() const { return m_mixer; }
	
	void           setSong(XMSong* p_song);
	inline XMSong* getSong() const { return m_song; }
	
	void play(bool p_looping = true, u8 p_startOrder = 0);
	void stop();
	
	bool processTick();
	
	void forceNextPattern(u8 p_pattern);
	inline u16 getCurrentPattern() const { return m_currentPattern; }
	inline u16 getCurrentRow()     const { return m_currentRow;     }
	void forceNextOrder(u8 p_order);
	
	void resetTempo();
	
	inline u8   getCurrentOrder()   const { return static_cast<u8>(m_currentOrder); }
	inline bool hasSongReachedEnd() const { return m_reachedEnd;                    }
	
private:
	struct XM_EnvelopeProcess
	{
		s16  tick;         // current tick
		u8   currentPoint; // current node where ticks belong to (for speed)
		u8   value;
		bool done;
	};
	
	struct XM_Channel
	{
		XM_EnvelopeProcess volumeEnvelope;
		XM_EnvelopeProcess panEnvelope;
		
		XMInstrument* instrumentPtr;
		XMSample *samplePtr;
		
		u32 noteStartOffset;
		
		s32 period;        // channel period
		s32 oldPeriod;     // period before note
		s32 tickrelPeriod; // only for this tick, relative period added to output
		s32 portaPeriod;   // target period for portamento
		
		u8 note;       // last note parsed
		u8 instrument; // last instrument parsed
		u8 command;
		u8 parameter;
		
		bool active;
		bool noteOff;
		u8 volume;
		u8 pan;
		
		u8 sample;
		bool portamentoActive;
		bool rowHasNote;
		bool restart;
		
		u32 restartOffset;
		
		s16 tickrelVolume;
		u16 fadeout;
		s16 realNote; // note + note adjustment in sample
		s8 finetune;  // finetune used
		u8 volumeCommand;
		u8 noteDelay;
		
		// effect memories
		u8 fx1Memory;
		u8 fx2Memory;
		u8 fx3Memory;
		u8 fx4Memory;
		s8 fx4VibratoPhase;
		u8 fx4VibratoSpeed;
		u8 fx4VibratoDepth;
		u8 fx4VibratoType;
		u8 fx5Memory;
		u8 fx6Memory;
		s8 fx7TremoloPhase;
		u8 fx7TremoloSpeed;
		u8 fx7TremoloDepth;
		u8 fx7TremoloType;
		u8 fxAMemory;
		u8 fxE1Memory;
		u8 fxE2Memory;
		u8 fxEAMemory;
		u8 fxEBMemory;
		u8 fxHMemory;
		u8 fxPMemory;
		u8 fxRMemory;
		u8 fxX1Memory;
		u8 fxX2Memory;
		
		u8 volfx6Memory;
		u8 volfx7Memory;
		u8 volfx8Memory;
		u8 volfx9Memory;
		u8 volfxAVibratoSpeed;
		s8 volfxBVibratoPhase;
		u8 volfxBVibratoDepth;
		u8 volfxDMemory;
		u8 volfxEMemory;
		u8 volfxFMemory;
	};
	
	struct XM_PatternDecompressor
	{
		u8  caches[XM_MaxChannels][5];
		u8* patternPtr;
		u8* posPtr;
		s16 row;
		s8  channel;
		s16 lastRow;     // last requested row
		s8  lastChannel; // last requested row
	};
	
	XMPlayer(const XMPlayer&);
	XMPlayer& operator=(const XMPlayer&);
	
	void   reset();
	u16    getPatternLength(u8 p_pattern);
	void   resetDecompressor();
	XMNote decompressNote(u8 p_row, u8 p_channel);
	void   envelopeReset(XM_EnvelopeProcess* p_env_process);
	u32    getPeriod(s16 p_note, s8 p_fine);
	u32    getFrequency(u32 p_period);
	void   processNotes();
	void   processEnvelope(XM_Channel* ch, XM_EnvelopeProcess* p_env_process, XMEnvelope* p_envelope);
	void   doVibrato(XM_Channel* ch);
	void   doTonePorta(XM_Channel* ch);
	void   doVolumeSlide(XM_Channel* ch);
	void   processEffectsAndEnvelopes();
	void   updateMixer();
	
	XMMixer*     m_mixer;
	XMSong*      m_song;
	
	static const u16 ms_amigaPeriodTable[12 * 8 + 1];
	static const u32 ms_linearFrequencyTable[768];
	static const u8  ms_fx4and7table[32];
	
	XM_Channel m_channel[XM_MaxChannels];
	
	u16 m_tickCounter;
	u16 m_currentSpeed;
	u16 m_currentTempo;
	u8 m_globalVolume;
	
	// Position
	u16 m_currentOrder;
	u16 m_currentRow;
	u16 m_currentPattern;
	u16 m_patternDelay;
	
	// flags
	
	bool m_forceNextOrder;      // force an order change, XM commands Dxx, Bxx
	u8   m_forcedNextOrder;     // order change
	u16  m_forcedNextOrderRow; // order change
	
	bool m_forceNextPattern;    // force a pattern change
	u8   m_forcedNextPattern;   // pattern change
	
	bool m_active;
	
	XM_PatternDecompressor m_decompressor;
	
	bool m_looping;
	bool m_reachedEnd;
};

} // namespace end
}
}

#endif // INC_AUDIO_CHIBI_XMPLAYER_H
