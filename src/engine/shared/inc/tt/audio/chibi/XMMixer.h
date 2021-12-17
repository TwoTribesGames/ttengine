#ifndef INC_TT_AUDIO_CHIBI_XMMIXER_H
#define INC_TT_AUDIO_CHIBI_XMMIXER_H

#include <cstddef>

#include <tt/audio/chibi/types.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMUtil.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace audio {
namespace chibi {

/** XM_Mixer:
  *
  * Chibi XM Play provides a default software mixer, but for portability issues, or taking advantage of 
  * a certain architecture or mixing hardware, the mixer can be reimplemented for any other backend.
  *
  * Methods tagged with *LOCK* mean that they must not be called directly by the user, 
  * unless locking/unlocking is performed before/after calling them, or calling from the same
  * thread as the process callback.
  */

// forward declarations
class XMPlayer;

class XMMixer
{
public:
	enum Restriction
	{
		Restriction_NoPingPongLoops = (1 << 8), /** Mixer can't do pingpong loops, loader will alloc extra memory for them */
		Restriction_NeedsEndPadding = (1 << 9), /** Mixer needs extra (zeroed) space at the end of the sample for interpolation to work */
		Restriction_MaxVoicesMask   = 0xFF      /** Maximum amount of simultaneous voices this mixer can do */
	};
	
	inline XMMixer()
	:
	m_player(0)
	{ }
	virtual ~XMMixer() {}
	
	virtual void setProcessCallbackInterval(u32 p_usec) = 0; /** set interval for process callback, in usecs (used by player, don't call) */
	
	virtual void voiceStart(u8 p_voice, XMSampleID p_sample, u32 p_offset) = 0; /** start offset in audio frames, *LOCK* */
	virtual void voiceStop(u8 p_voice) = 0; /** stop voice, *LOCK* */
	virtual void voiceSetVolume(u8 p_voice, u8 p_vol) = 0; /** volume from 0 to 255 *LOCK* */
	virtual void voiceSetPan(u8 p_voice, u8 p_pan) = 0; /** pan from 0 to 255, 127 is center *LOCK* */
	virtual void voiceSetSpeed(u8 p_voice, u32 p_hz) = 0; /** speed, in audio frames per second *LOCK* */
	
	virtual bool voiceIsActive(u8 p_voice) = 0; /** speed, in audio frames/second *LOCK* */
	
	virtual XMSampleID sampleRegister(XMSampleData* p_sample_data) = 0; /** Mixer takes ownership of the sample data */
	virtual void sampleUnregister(XMSampleID p_sample) = 0; /** Mixer releases ownership of the sample, and may free the sample data */
	virtual void resetVoices() = 0; /** silence and reset all voices */
	virtual void resetSamples() = 0; /** unregister all samples */
	
	virtual u32 getRestrictions() = 0; /** Mixer restrictions bits, check MixerRestrictionsFlags */
	
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	
	virtual void setVolume(real p_volume) = 0;
	
	virtual bool update() = 0;
	
	inline static void* operator new(std::size_t p_blockSize)
	{
		void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_blockSize), XMMemoryManager::AllocType_SWMixer);
		if ( mem != 0 )
		{
			XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_blockSize));
		}
		return mem;
	}
	
	inline static void operator delete(void* p_block)
	{
		XMUtil::getMemoryManager()->free(p_block, XMMemoryManager::AllocType_SWMixer);
	}
	
	inline void      setPlayer(XMPlayer* p_player) { m_player = p_player; }
	inline XMPlayer* getPlayer() const             { return m_player;     }
	
protected:
	XMPlayer* m_player;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMMIXER_H
