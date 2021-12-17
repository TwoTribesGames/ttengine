#ifndef INC_TT_AUDIO_CHIBI_XMSOFTWAREMIXER_H
#define INC_TT_AUDIO_CHIBI_XMSOFTWAREMIXER_H

#include <cstddef>

#include <tt/audio/chibi/types.h>
#include <tt/audio/chibi/XMMixer.h>


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

class XMSoftwareMixer : public XMMixer
{
public:
	/*******************************
	*********SOFTWARE MIXER********
	*******************************/
	
	/* Software Mixer Data */
	
	/* change this limit as you see fit */
	#define _XM_SW_MAX_SAMPLES 256
	#define _XM_SW_FRAC_SIZE 12
	#define _XM_SW_VOL_FRAC_SIZE 8
	#define DECLICKER_BUFFER_BITS 6
	#define DECLICKER_BUFFER_SIZE (1<<DECLICKER_BUFFER_BITS)
	#define DECLICKER_BUFFER_MASK (DECLICKER_BUFFER_SIZE-1)
	#define DECLICKER_FADE_BITS 5
	#define DECLICKER_FADE_SIZE (1<<DECLICKER_FADE_BITS)
	
	struct XMSoftwareMixerVoice
	{
		XMSampleID sample;
		s32 incrementFp;
		
		s32 oldvolR,oldvolL; // volume ramp history
		s64 offset;
		
		bool start; // mixing for the first time with new parameters
		bool active;
		u8 volume; // 0 .. 255
		u8 pan;  // 0 .. 255
	};
	
	XMSoftwareMixer(u32 p_samplingRateHz, u8 p_maxChannels);
	virtual ~XMSoftwareMixer();
	
	virtual void setProcessCallbackInterval(u32 p_usec); /** set interval for process callback, in usecs (used by player, don't call) */
	
	virtual void voiceStart(u8 p_voice, XMSampleID p_sample, u32 p_offset); /** start offset in audio frames, *LOCK* */
	virtual void voiceStop(u8 p_voice); /** stop voice, *LOCK* */
	virtual void voiceSetVolume(u8 p_voice, u8 p_vol); /** volume from 0 to 255 *LOCK* */
	virtual void voiceSetPan(u8 p_voice, u8 p_pan); /** pan from 0 to 255, 127 is center *LOCK* */
	virtual void voiceSetSpeed(u8 p_voice, u32 p_hz); /** speed, in audio frames per second *LOCK* */
	
	virtual bool voiceIsActive(u8 p_voice); /** speed, in audio frames/second *LOCK* */
	
	virtual XMSampleID sampleRegister(XMSampleData* p_sample_data); /** Mixer takes ownership of the sample data */
	virtual void sampleUnregister(XMSampleID p_sample); /** Mixer releases ownership of the sample, and may free the sample data */
	virtual void resetVoices(); /** silence and reset all voices */
	virtual void resetSamples(); /** unregister all samples */
	
	virtual u32 getRestrictions(); /** Mixer restrictions bits, check MixerRestrictionsFlags */
	
	u32 mixToBuffer(s32* p_buffer, u32 p_frames); /** Software mix to buffer, in stereo interleaved, 32 bits per sample. Mix the amount of frames requested. THIS ONLY WORKS FOR AN EXISTING AND ACTIVE SOFTWARE MIXER */
	
	static void* operator new(std::size_t p_blockSize);
	static void  operator delete(void* p_block);
	
private:
	void mixVoiceToBuffer(u8 p_voice, s32* p_buffer,u32 p_frames);
	void mixVoicesToBuffer(s32* p_buffer, u32 p_frames);
	
	// No copying
	XMSoftwareMixer(const XMSoftwareMixer&);
	XMSoftwareMixer& operator=(const XMSoftwareMixer&);
	
	
	XMSampleData          m_samplePool[_XM_SW_MAX_SAMPLES];
	XMSoftwareMixerVoice* m_voices;
	
	s32* m_mixdownBuffer;
	u32  m_samplingRate;
	
	void (*m_processCallback)();
	u32 m_callbackInterval;
	u32 m_callbackIntervalCountdown;
	
	s32 m_declickerRbuffer[DECLICKER_BUFFER_SIZE*2];
	s32 m_declickerFade[DECLICKER_FADE_SIZE*2];
	u32 m_declickerPos;
	
	u8 m_maxVoices;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMSOFTWAREMIXER_H
