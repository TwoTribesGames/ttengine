#include <tt/audio/chibi/XMPlayer.h>
#include <tt/audio/chibi/XMSoftwareMixer.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace chibi {

XMSoftwareMixer::XMSoftwareMixer(u32 p_samplingRateHz, u8 p_maxChannels)
:
m_voices((XMSoftwareMixerVoice*)XMUtil::getMemoryManager()->alloc(sizeof(XMSoftwareMixerVoice) * p_maxChannels, XMMemoryManager::AllocType_SWMixer)),
m_mixdownBuffer(0),
m_samplingRate(p_samplingRateHz),
m_processCallback(0),
m_callbackInterval(0),
m_callbackIntervalCountdown(0),
m_declickerPos(0),
m_maxVoices(p_maxChannels)
{
	for (s32 i = 0; i < DECLICKER_BUFFER_SIZE * 2; ++i)
	{
		m_declickerRbuffer[i] = 0;
	}
	for (s32 i = 0; i < DECLICKER_FADE_SIZE * 2; ++i)
	{
		m_declickerFade[i] = 0;
	}
	
	resetVoices();
}


XMSoftwareMixer::~XMSoftwareMixer()
{
	XMUtil::getMemoryManager()->free(m_voices, XMMemoryManager::AllocType_SWMixer);
}


void XMSoftwareMixer::voiceStart(u8 p_voice, XMSampleID p_sample, u32 p_offset)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return;
	}
	
	if (p_sample < 0 || p_sample >= _XM_SW_MAX_SAMPLES || m_samplePool[p_sample].data == 0)
	{
		TT_WARN("INVALID SAMPLE: %i\n", p_voice);
		return;
	}
	
	// SEND CURRENT VOICE, IF ACTIVE TO DECLICKER
	
	if (m_voices[p_voice].active)
	{
		voiceStop(p_voice);
	}
	
	// Set and Validate new voice
	
	if (p_offset >= m_samplePool[p_sample].length)
	{
		// turn off voice, offset is too long
		m_voices[p_voice].active = false;
		return;
	}
	
	m_voices[p_voice].sample      = p_sample;
	m_voices[p_voice].offset      = p_offset;
	m_voices[p_voice].offset    <<= _XM_SW_FRAC_SIZE; // convert to fixed point
	m_voices[p_voice].start       = true;
	m_voices[p_voice].active      = true;
	m_voices[p_voice].incrementFp = 0;
}


void XMSoftwareMixer::voiceStop(u8 p_voice)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return;
	}
	
	if (m_voices[p_voice].active == false)
	{
		return;
	}
	
	if (m_voices[p_voice].incrementFp != 0)
	{
		for (int i =0; i < (DECLICKER_FADE_SIZE * 2); ++i)
		{
			m_declickerFade[i] = 0;
		}
		
		mixVoiceToBuffer(p_voice, &m_declickerFade[0], DECLICKER_FADE_SIZE);
		
		for (int i = 0; i < DECLICKER_FADE_SIZE; ++i)
		{
			int dpos = (int)(((m_declickerPos + i) & DECLICKER_BUFFER_MASK) << 1);
			int inv = DECLICKER_FADE_SIZE - i;
			
			m_declickerRbuffer[dpos]     += (m_declickerFade[i << 1]       >> DECLICKER_FADE_BITS) * inv;
			m_declickerRbuffer[dpos + 1] += (m_declickerFade[(i << 1) + 1] >> DECLICKER_FADE_BITS) * inv;
		}
	}
	
	// SEND CURRENT VOICE, IF ACTIVE TO DECLICKER
	m_voices[p_voice].active = false;
}


void XMSoftwareMixer::voiceSetVolume(u8 p_voice, u8 p_vol)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return;
	}
	
	if (m_voices[p_voice].active == false)
	{
		return;
	}
	
	m_voices[p_voice].volume = p_vol;
}


void XMSoftwareMixer::voiceSetPan(u8 p_voice, u8 p_pan)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return;
	}
	
	if (m_voices[p_voice].active == false)
	{
		return;
	}
	
	m_voices[p_voice].pan = p_pan;
}


bool XMSoftwareMixer::voiceIsActive(u8 p_voice)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return false;
	}
	
	return m_voices[p_voice].active;
}


void XMSoftwareMixer::voiceSetSpeed(u8 p_voice, u32 p_hz)
{
	if (p_voice >= m_maxVoices)
	{
		TT_WARN("INVALID VOICE: %i\n", p_voice);
		return;
	}
	
	if (m_voices[p_voice].active == false)
	{
		return;
	}
	
	// incrementor in fixed point
	bool backwards = (m_voices[p_voice].incrementFp < 0);
	m_voices[p_voice].incrementFp = (s32)(((s64)p_hz << _XM_SW_FRAC_SIZE) / m_samplingRate);
	if (backwards)
	{
		m_voices[p_voice].incrementFp = -m_voices[p_voice].incrementFp;
	}
}


XMSampleID XMSoftwareMixer::sampleRegister(XMSampleData* p_sampleData)
{
	if (p_sampleData->data == 0)
	{
		TT_WARN("SAMPLE DATA IS NULL");
		return XMConstant_InvalidSampleID;
	}
	
	for (int i = 0; i < _XM_SW_MAX_SAMPLES; ++i)
	{
		if (m_samplePool[i].data == p_sampleData)
		{
			TT_WARN("SAMPLE ALREADY REGISTERED");
			return XMConstant_InvalidSampleID;
		}
	}
	
	for (int i = 0; i < _XM_SW_MAX_SAMPLES; ++i)
	{
		if (m_samplePool[i].data != 0)
		{
			continue;
		}
		
		m_samplePool[i] = *p_sampleData;
		return i;
	}
	
	TT_WARN("SAMPLE POOL FULL");
	return XMConstant_InvalidSampleID;
}


void XMSoftwareMixer::sampleUnregister(XMSampleID p_sample)
{
	if (p_sample < 0 || p_sample >= _XM_SW_MAX_SAMPLES)
	{
		TT_WARN("INVALID SAMPLE ID: %i", p_sample);
		return;
	}
	
	XMUtil::getMemoryManager()->free(m_samplePool[p_sample].data, XMMemoryManager::AllocType_Sample);
	m_samplePool[p_sample].data = 0; // set to unused
}


void XMSoftwareMixer::resetVoices()
{
	for (int i = 0; i < m_maxVoices; ++i)
	{
		XMSoftwareMixerVoice& v = m_voices[i];
		
		v.sample      = XMConstant_InvalidSampleID;
		v.incrementFp = 0;
		
		v.oldvolR = 0;
		v.oldvolL = 0;
		v.offset  = 0;
		
		v.start  = false;
		v.active = false;
		v.volume = 0;
		v.pan    = 0;
	}
}


void XMSoftwareMixer::resetSamples()
{
	// does nothing
}


u32 XMSoftwareMixer::getRestrictions()
{
	return (Restriction_NeedsEndPadding | (u32)m_maxVoices);
}


void XMSoftwareMixer::setProcessCallbackInterval(u32 p_usec)
{
	// convert to frames
	s64 interval = p_usec;
	interval *= m_samplingRate;
	interval /= 1000000L;
	
	m_callbackInterval = (u32)interval;
}


u32 XMSoftwareMixer::mixToBuffer(s32* p_buffer, u32 p_frames)
{
	if (m_player == 0)
	{
		return 0;
	}
	
	u32 todo = p_frames;
	bool doResume = true;
	while (doResume && todo != 0)
	{
		u32 toMix = 0;
		
		if (m_callbackIntervalCountdown == 0)
		{
			// callback time!
			
			doResume = m_player->processTick(); // pass the ball to the XMPlayer
			m_callbackIntervalCountdown = m_callbackInterval;
		}
		
		toMix = (m_callbackIntervalCountdown < todo) ? m_callbackIntervalCountdown : todo;
		
		mixVoicesToBuffer(p_buffer, toMix);
		p_buffer += toMix * 2; // advance the pointer too
		
		m_callbackIntervalCountdown -= toMix;
		todo -= toMix;
	}
	
	if (doResume)
	{
		return p_frames - todo;
	}
	else
	{
		return 0;
	}
}


void* XMSoftwareMixer::operator new(std::size_t p_blockSize)
{
	// get memory manager
	void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_blockSize), XMMemoryManager::AllocType_SWMixer);
	if (mem != 0)
	{
		XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_blockSize));
	}
	return mem;
}


void XMSoftwareMixer::operator delete(void* p_block)
{
	// get memory manager
	XMUtil::getMemoryManager()->free(p_block, XMMemoryManager::AllocType_SWMixer);
}


// Private functions

void XMSoftwareMixer::mixVoiceToBuffer(u8 p_voice, s32* p_buffer,u32 p_frames)
{
	XMSoftwareMixerVoice& v = m_voices[p_voice];
	XMSampleData* s = &m_samplePool[v.sample];
	
	u32 vol = v.volume; // 32 bits version of the volume
	
	// some 64-bit fixed point precaches
	s64 loopBeginFp = ((s64)s->loopBegin << _XM_SW_FRAC_SIZE);
	s64 loopEndFp   = ((s64)s->loopEnd << _XM_SW_FRAC_SIZE);
	s64 lengthFp    = ((s64)s->length << _XM_SW_FRAC_SIZE);
	s64 beginLimit  = (s->loopType != XMLoopType_Disabled) ? loopBeginFp : 0;
	s64 endLimit    = (s->loopType != XMLoopType_Disabled) ? loopEndFp : lengthFp;
	s32 todo        = (s32)p_frames;
	
	// check that sample is valid
	if (s->data == 0)
	{
		// if sample dissapeared, disable voice
		v.active = false;
		return;
	}
	
	// compute voice left and right volume multipliers
#ifdef USE_OLD_PANNING
	// Center panning means 50% left, 50% right
	s32 dstVolL = static_cast<s32>((vol * (255 - v.pan)) >> 8);
	s32 dstVolR = static_cast<s32>((vol * v.pan) >> 8);
#else
	// Center panning means 100% left, 100% right
	s32 dstVolL = static_cast<s32>(vol);
	s32 dstVolR = static_cast<s32>(vol);
	
	if (v.pan > 128)
	{
		// pan right
		dstVolL = static_cast<s32>((vol * (255 - v.pan)) >> 7);
	}
	else
	{
		// pan left
		dstVolR = static_cast<s32>((vol * v.pan) >> 7);
	}
#endif
	
	TT_WARNING(dstVolL >= 0 && dstVolL <= 255, "Voice %d, left channel volume %d out of bounds 0-255", p_voice, dstVolL);
	TT_WARNING(dstVolR >= 0 && dstVolR <= 255, "Voice %d, right channel volume %d out of bounds 0-255", p_voice, dstVolR);
	
	if (v.start)
	{
		// first time, reset ramp
		v.oldvolL = dstVolL;
		v.oldvolR = dstVolR;
	}
	
	// compute volume ramps, use fixed point calculations
	s32 volL = v.oldvolL << _XM_SW_VOL_FRAC_SIZE; // shift up to fixed point
	s32 volR = v.oldvolR << _XM_SW_VOL_FRAC_SIZE;
	s32 volLInc = (s32)(((dstVolL - v.oldvolL) << _XM_SW_VOL_FRAC_SIZE) / p_frames);
	s32 volRInc = (s32)(((dstVolR - v.oldvolR) << _XM_SW_VOL_FRAC_SIZE) / p_frames);
	
	// @TODO validar loops al registrar , pedir un poco mas de memoria para interpolar
	// @TODO: translate
	
	while (todo > 0)
	{
		s64 limit  = 0;
		s32 target = 0;
		s32 aux    = 0;
		
		// LOOP CHECKING
		
		if (v.incrementFp < 0)
		{
			// going backwards
			if (s->loopType != XMLoopType_Disabled && v.offset < loopBeginFp)
			{
				// loopstart reached
				if (s->loopType == XMLoopType_PingPong)
				{
					// bounce ping pong
					v.offset = loopBeginFp + (loopBeginFp - v.offset);
					v.incrementFp = -v.incrementFp;
				}
				else
				{
					// go to loop-end
					v.offset = loopEndFp - (loopBeginFp - v.offset);
				}
			}
			else
			{
				// check for sample not reaching begining
				if (v.offset < 0)
				{
					v.active = false;
					break;
				}
			}
		}
		else
		{
			/* going forward */
			if (s->loopType != XMLoopType_Disabled && v.offset >= loopEndFp)
			{
				// loopend reached
				if (s->loopType == XMLoopType_PingPong)
				{
					// bounce ping pong
					v.offset = loopEndFp - (v.offset - loopEndFp);
					v.incrementFp = -v.incrementFp;
				}
				else
				{
					// go to loop-begin
					v.offset = loopBeginFp + (v.offset - loopEndFp);
				}
			}
			else
			{
				// no loop, check for end of sample
				if (v.offset >= lengthFp)
				{
					v.active = false;
					break;
				}
			}
		}
		
		// MIXCOUNT COMPUTING
		
		// next possible limit (looppoints or sample begin/end
		limit = (v.incrementFp < 0) ? beginLimit : endLimit;
		
		// compute what is shorter, the todo or the limit?
		aux = (s32)((limit - v.offset) / v.incrementFp + 1);
		target = (aux < todo) ? aux : todo; // mix target is the shorter buffer
		
		// check just in case
		if (target <= 0)
		{
			v.active = false;
			break;
		}
		
		todo -= target;
		
		switch (s->format)
		{
		case XMSampleFormat_PCM8: // signed 8-bits
			{
				// convert to local mixing chunk so
				// 32 bits resampling can be used
				s8* srcPtr = &((s8*)s->data)[v.offset >> _XM_SW_FRAC_SIZE];
				s32 offset = (s32)(v.offset & ((1 << _XM_SW_FRAC_SIZE) - 1)); // strip integer
				
				while (target--)
				{
					s32 val     = srcPtr[offset >> _XM_SW_FRAC_SIZE];       // scale up to fixed point
					s32 valNext = srcPtr[(offset >> _XM_SW_FRAC_SIZE) + 1]; // scale up to fixed point
					val <<= 8;     // convert to 16 bit sample
					valNext <<= 8; // convert to 16 bit sample
					
					// linear interpolation of samples
					val = val + ((valNext - val) * ((offset) & (((1 << _XM_SW_FRAC_SIZE)) - 1)) >> _XM_SW_FRAC_SIZE);
					
					*(p_buffer++) += val * (volL >> _XM_SW_VOL_FRAC_SIZE);
					*(p_buffer++) += val * (volR >> _XM_SW_VOL_FRAC_SIZE);
					
					// linear interpolation of volume
					volL += volLInc;
					volR += volRInc;
					offset += v.incrementFp;
				}
				
				v.offset += offset;
			}
			break;
			
		case XMSampleFormat_PCM16: // signed 16-bits
		{
			// convert to local mixing chunk so 
			// 32 bits resampling can be used
			s16* srcPtr = &((s16*)s->data)[v.offset >> _XM_SW_FRAC_SIZE];
			s32  offset = (s32)(v.offset & ((1 << _XM_SW_FRAC_SIZE) - 1)); // strip integer
			
			while (target--)
			{
				s32 val     = srcPtr[offset >> _XM_SW_FRAC_SIZE];
				s32 valNext = srcPtr[(offset >> _XM_SW_FRAC_SIZE) + 1];
				
				val = val + ((valNext - val) * ((offset) & (((1 << _XM_SW_FRAC_SIZE)) - 1)) >> _XM_SW_FRAC_SIZE);
				
				*(p_buffer++) += val * (volL >> _XM_SW_VOL_FRAC_SIZE);
				*(p_buffer++) += val * (volR >> _XM_SW_VOL_FRAC_SIZE);
				
				volL += volLInc;
				volR += volRInc;
				
				offset += v.incrementFp;
			}
			
			v.offset += offset;
			break;
		}
		case XMSampleFormat_IMA_ADPCM: /* ima-adpcm */
		{
			/* Not supported in software yet */
			break;
		}
		case XMSampleFormat_Custom: /* Custom format, XM_Mixer should support this */
		{
			/* I can't play this! */
			break;
		}
		}
	}
	
	v.oldvolL = dstVolL;
	v.oldvolR = dstVolR;
}


void XMSoftwareMixer::mixVoicesToBuffer(s32* p_buffer, u32 p_frames)
{
	for (u8 i = 0; i < m_maxVoices; ++i)
	{
		if (m_voices[i].active == false) // ignore inactive voices
		{
			continue;
		}
		
		mixVoiceToBuffer(i, p_buffer, p_frames);
	}
	
	static const s32 minValue = -32768;
	static const s32 maxValue =  32767;
	
	// This variable determines how far the actual result of the mixing gets
	// shifted down before clipping. This is to reduce clipping.
	static const s32 s_volumeReductionShift = 1;
	
	s32* targetBuff = p_buffer;
	
	for (u32 i = 0; i < p_frames; ++i)
	{
		u32 dpos = (m_declickerPos & DECLICKER_BUFFER_MASK) << 1;
		
		// apply declicker, scale down, and perform clipping
		*targetBuff += m_declickerRbuffer[dpos];
		*targetBuff >>= 8 + s_volumeReductionShift;
		if (*targetBuff < minValue) *targetBuff = minValue;
		if (*targetBuff > maxValue) *targetBuff = maxValue;
		++targetBuff;
		
		*targetBuff += m_declickerRbuffer[dpos + 1];
		*targetBuff >>= 8 + s_volumeReductionShift;
		if (*targetBuff < minValue) *targetBuff = minValue;
		if (*targetBuff > maxValue) *targetBuff = maxValue;
		++targetBuff;
		
		m_declickerRbuffer[dpos]      = 0;
		m_declickerRbuffer[dpos + 1 ] = 0;
		++m_declickerPos;
	}
}


} // namespace end
}
}
