/* Chibi XM Play  - Copyright (c) 2007, Juan Linietsky */
/*

License:

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
            
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <tt/audio/chibi/XMInstrument.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMMixer.h>
#include <tt/audio/chibi/XMPlayer.h>
#include <tt/audio/chibi/XMSong.h>


namespace tt {
namespace audio {
namespace chibi {


#define _XM_ABS(m_v)  (((m_v) < 0) ? (-(m_v)) : (m_v))


XMPlayer::XMPlayer()
:
m_mixer(0),
m_song(0),
m_tickCounter(0),
m_currentSpeed(0),
m_currentTempo(0),
m_globalVolume(0),
m_currentOrder(0),
m_currentRow(0),
m_currentPattern(0),
m_patternDelay(0),
m_forceNextOrder(false),
m_forcedNextOrder(0),
m_forcedNextOrderRow(0),
m_forceNextPattern(false),
m_forcedNextPattern(0),
m_active(false),
m_decompressor(),
m_looping(false),
m_reachedEnd(false)
{
}


void* XMPlayer::operator new(std::size_t p_size)
{
	void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_size), XMMemoryManager::AllocType_Player);
	if (mem != 0)
	{
		XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_size));
	}
	return mem;
}


void* XMPlayer::operator new[](std::size_t p_size)
{
	void* mem = XMUtil::getMemoryManager()->alloc(static_cast<u32>(p_size), XMMemoryManager::AllocType_Player);
	if (mem != 0)
	{
		XMUtil::getMemoryManager()->zeroMem(mem, static_cast<u32>(p_size));
	}
	return mem;
}


void XMPlayer::operator delete(void* p_mem)
{
	XMUtil::getMemoryManager()->free(p_mem, XMMemoryManager::AllocType_Player);
}


void XMPlayer::operator delete[](void* p_mem)
{
	XMUtil::getMemoryManager()->free(p_mem, XMMemoryManager::AllocType_Player);
}


void XMPlayer::reset()
{
	for (int i = 0; i < XM_MaxChannels; ++i)
	{
		XMUtil::getMemoryManager()->zeroMem(&m_channel[i], sizeof(XM_Channel));
		m_channel[i].pan = 32;
	}
	
	if (m_song != 0)
	{
		m_currentSpeed = m_song->speed;
		m_currentTempo = m_song->tempo;
	}
	else
	{
		m_currentSpeed = 5;
		m_currentTempo = 120;
		
	}
	
	m_tickCounter = m_currentSpeed; // so it starts on a row
	
	// Position
	m_currentOrder = 0;
	m_currentRow   = 0;
	m_patternDelay = 0;
	
	if (m_song != 0)
	{
		m_currentPattern = m_song->orderList[0];
	}
	
	m_forceNextOrder     = false;
	m_forcedNextOrder    = 0;
	m_forcedNextOrderRow = 0;
	m_active             = false;
	m_globalVolume       = 64; // 0x40
	XMUtil::getMemoryManager()->zeroMem(&m_decompressor, sizeof(XM_PatternDecompressor));
	
	m_reachedEnd         = false;
}


u16 XMPlayer::getPatternLength(u8 p_pattern)
{
	if (p_pattern >= m_song->patternCount || m_song->patternData[p_pattern] == 0)
	{
		return 64;
	}
	else 
	{
		return (u16)(*m_song->patternData[p_pattern] + 1);
	}
}


void XMPlayer::resetDecompressor()
{
	if (m_decompressor.patternPtr != 0)
	{
		// reset caches
		for (int i = 0; i < XM_MaxChannels; ++i)
		{
			m_decompressor.caches[i][0] = XM_FieldEmpty;
			m_decompressor.caches[i][1] = XM_FieldEmpty;
			m_decompressor.caches[i][2] = XM_FieldEmpty;
			m_decompressor.caches[i][3] = XM_FieldEmpty;
			m_decompressor.caches[i][4] = 0; // only values other than 0 are read for this as cache
		}
		
		// set stream pointer
		
		m_decompressor.posPtr = &m_decompressor.patternPtr[1];
	}
	else
	{
		m_decompressor.posPtr = 0;
	}
	
	// set stream pos
	m_decompressor.row        = 0;
	m_decompressor.channel    = 0;
	m_decompressor.lastRow    = 0;
	m_decompressor.lastChannel= 0;
}


static const XMNote s_emptyNote;

XMNote XMPlayer::decompressNote(u8 p_row, u8 p_channel)
{
	XMNote n              = s_emptyNote;
	u8* currentPatternPtr = 0;
	
	// check current pattern validity
	if (m_currentPattern < m_song->patternCount && m_song->patternData[m_currentPattern] != 0)
	{
		currentPatternPtr = m_song->patternData[m_currentPattern];
	}
	
	// if pattern changed, reset decompressor
	if (currentPatternPtr != m_decompressor.patternPtr)
	{
		m_decompressor.patternPtr = currentPatternPtr;
		resetDecompressor();
	}
	
	// check for empty pattern, return empty note if pattern is empty
	
	if (currentPatternPtr == 0)
	{
		return s_emptyNote;
	}
	
	// check if the position is requested is behind the seekpos,
	// and not after. If so, reset decompressor and seek to begining
	
	if (m_decompressor.lastRow > p_row ||
	    (m_decompressor.lastRow == p_row && m_decompressor.lastChannel > p_channel))
	{
		resetDecompressor();
	}
	
	while (m_decompressor.row < p_row || (m_decompressor.row == p_row && m_decompressor.channel <= p_channel))
	{
		u8 cmd = *m_decompressor.posPtr;
		
		// at end of pattern, just break
		if ((cmd >> 5) == XM_CompEndOfPattern)
		{
			break;
		}
		
		switch (cmd >> 5)
		{
		case XM_CompReadChannelAdvRow:
			m_decompressor.row++;
			// FALLTHROUGH
			
		case XM_CompSetChannel:
			{
				m_decompressor.channel = (s8)(cmd & 0x1F); // channel in lower 5 bits
			}
			break;
			
		case XM_CompReadFields: 
			// read fields into the cache
			for (int i = 0; i < 5; ++i)
			{
				if ((cmd & (1 << i)) != 0)
				{
					m_decompressor.posPtr++;
					m_decompressor.caches[m_decompressor.channel][i] = *m_decompressor.posPtr;
				}
			}
			// FALLTHROUGH because values must be read
			
		case XM_CompUseCaches:
			{
				// if not the same position, break
				if (m_decompressor.row != p_row || m_decompressor.channel != p_channel)
				{
					break;
				}
				
				// otherwise assign the caches to the note
				
				if ((cmd & XM_CompNoteBit) != 0)
				{
					n.note = m_decompressor.caches[p_channel][0];
				}
				if ((cmd & XM_CompInstrumentBit) != 0)
				{
					n.instrument = m_decompressor.caches[p_channel][1];
				}
				if ((cmd & XM_CompVolumeBit) != 0)
				{
					n.volume = (u8)(m_decompressor.caches[p_channel][2] + 0x10);
				}
				if ((cmd & XM_CompCommandBit) != 0)
				{
					n.command = m_decompressor.caches[p_channel][3];
				}
				if ((cmd & XM_CompParameterBit) != 0)
				{
					n.parameter = m_decompressor.caches[p_channel][4];
				}
			}
			break;
			
		case XM_CompEndOfPattern:
			{
				TT_PANIC("Code should never reach here!");
			}
			break;
			
		case XM_CompAdvanceRows:
			{
				m_decompressor.row    += (cmd & 0x1F) + 1;
				m_decompressor.channel = 0;
			}
			break;
			
		default:
			TT_PANIC("INVALID COMMAND!");
		}
		
		// if not at end of pattern, advance one byte
		m_decompressor.posPtr++;
	}
	
	m_decompressor.lastRow     = p_row;
	m_decompressor.lastChannel = (s8)p_channel;
	
	return n;
}


void XMPlayer::envelopeReset(XM_EnvelopeProcess* p_envProcess)
{
	p_envProcess->tick         = 0;
	p_envProcess->currentPoint = 0;
	p_envProcess->value        = 0;
	p_envProcess->done         = false;
}


u32 XMPlayer::getPeriod(s16 p_note, s8 p_fine)
{
	// Get period, specified by fasttracker xm.txt
	
	if ((m_song->flags & XMSong::Flag_LinearPeriods) != 0)
	{
		s32 period = 10 * 12 * 16 * 4 - (s32)(p_note-1) * 16 * 4 - (s32)p_fine / 2;
		
		if (period < 0)
		{
			period = 0;
		}
		
		return (u32)period;
	}
	else
	{
		// amiga periods
		
		s16 fine = p_fine;
		s16 note = p_note;
		u16 idx; // index in amiga table
		s16 period;
		
		// fix to fit table, note always positive and fine always 0 .. 127
		if (fine < 0)
		{
			fine += 128;
			--note;
		}
		
		if (note < 0)
		{
			note = 0;
		}
		
		// find index in table
		idx = (u16)(((note % 12) << 3) + (fine >> 4));
		
		period = (s16)ms_amigaPeriodTable[idx];
		
		// interpolate for further fine-ness :)
		period += ((fine & 0xF) * (ms_amigaPeriodTable[idx + 1] - period)) >> 4;
		
		// apply octave
		period = (s16)(((period << 4) / (1 << (note / 12))) << 1);
		return (u32)period;
	}
}


u32 XMPlayer::getFrequency(u32 p_period)
{
	// Get frequency, specified by fasttracker xm.txt
	
	if ((m_song->flags & XMSong::Flag_LinearPeriods) != 0)
	{
		s32 shift = (((s32)p_period / 768) - 0);
		
		if (shift > 0)
		{
			return ms_linearFrequencyTable[p_period % 768] >> shift;
		}
		else
		{
			return ms_linearFrequencyTable[p_period % 768] << (-shift);
		}
	}
	else
	{
		// amiga
		
		return 8363 * 1712 / p_period;
	}
}


void XMPlayer::processNotes()
{
	int channels = (m_song->flags & XMSong::Flag_ChannelsUsedMask) + 1;
	
	for (int i = 0; i < channels; ++i)
	{
		// Decomrpess a Note
		XM_Channel* ch   = &m_channel[i];
		XMNote note      = decompressNote((u8)(m_currentRow), (u8)i);
		bool processNote = false;
		
		// Validate instrument and note fields
		
		if ((note.instrument != XM_FieldEmpty && 
		    (note.instrument >= m_song->instrumentCount ||
		     m_song->instrumentData[note.instrument] == 0)) ||
		    (note.note!=XM_FieldEmpty && note.note>XM_NoteOff))
		{
			// if any is invalid, both become invalid
			note.instrument = XM_FieldEmpty;
			note.note       = XM_FieldEmpty;
		}
		
		// Determine wether note should be processed
		
		if ((note.note != XM_FieldEmpty || note.instrument != XM_FieldEmpty) && note.note != XM_NoteOff)
		{
			if (note.note == XM_FieldEmpty)
			{
				// if note field is empty, there is only one case where
				// a note can be played.. either the channel must be inactive,
				// or the instrment is different than the one previously used
				// in the channel. If the conditions meet, the previos note played
				// is used (as long as it exist)
				if ((ch->active == false || ch->instrument != note.instrument) &&
				    ch->note != XM_FieldEmpty)
				{
					note.note   = ch->note; // use previous channel note
					processNote = true;
				}
			}
			else if (note.instrument == XM_FieldEmpty)
			{
				if (note.note == XM_NoteOff)
				{
					processNote = true;
				}
				if ( ch->instrument != XM_FieldEmpty )
				{
					note.instrument = ch->instrument;
					processNote     = true;
				}
			}
			else
			{
				processNote = true;
			}
			
			if (processNote)
			{
				// check the sample/instrument combination for validity
				
				u8 sample;
				// this was validated before...
				
				XMInstrument* ins = m_song->instrumentData[note.instrument];
				u8 index          = static_cast<u8>(note.note - 1);
				sample            = ins->noteSample[index >> 1];
				sample            = (u8)((index & 1) ? sample >> 4 : sample & 0xF);
				
				if (sample >= ins->sampleCount ||
				    ins->samples[sample].sampleId == XMConstant_InvalidSampleID)
				{
					processNote = false; // invalid sample
				}
				
			}
		}
		
		if (processNote)
		{
			// set some note-start flags
			ch->noteStartOffset  = 0;     // unless changed by effect
			ch->portamentoActive = false; // unless changed by effect
			ch->rowHasNote       = true;
			ch->noteOff          = false;
			
			ch->note = note.note;
			
			// all this was previously validated
			ch->instrument    = note.instrument;
			ch->instrumentPtr = m_song->instrumentData[ch->instrument];
			
			// extract sample nibble
			u8 index = static_cast<u8>(ch->note - 1);
			u8 sample = ch->instrumentPtr->noteSample[index >> 1];
			sample = (u8)((index & 1) ? sample >> 4 : sample & 0xF);
			
			bool sampleChanged = ch->sample != sample;
			ch->sample    = sample;
			ch->samplePtr = &ch->instrumentPtr->samples[sample];
			
			ch->realNote = (s16)(ch->note + ch->samplePtr->baseNote);
			ch->finetune = ch->samplePtr->finetune;
			
			// envelopes
			envelopeReset(&ch->volumeEnvelope);
			envelopeReset(&ch->panEnvelope);
			
			// get period from note
			ch->oldPeriod = ch->period;
			ch->period    = (s32)getPeriod(ch->realNote, ch->samplePtr->finetune);
			
			if (sampleChanged || ch->oldPeriod == 0)
			{
				// if sample changed, fix portamento period
				ch->oldPeriod = ch->period; 
			}
			
			if (ch->period == 0)
			{
				ch->active = false;
			}
			
			// volume/pan
			
			ch->volume = ch->samplePtr->volume; // may be overriden by volume column anyway
			ch->pan    = ch->samplePtr->pan;    // may be overriden by volume column anyway
			
			ch->restart       = true;
			ch->restartOffset = 0;    // unless changed by command
			ch->active        = true; // may get unset later
			ch->fadeout       = 0xFFFF;
		}
		else
		{
			ch->rowHasNote = false;
		}
		
		// process note off
		if (note.note == XM_NoteOff && ch->active)
		{
			// if channels is not active, ignore
			
			if (ch->instrumentPtr != 0)
			{
				if ((ch->instrumentPtr->volumeEnvelope.flags & XMEnvelope::Flag_Enabled) != 0)
				{
					// if the envelope is enabled, noteoff is used, for both
					// the envelope and the fadeout
					ch->noteOff = true;
				}
				else
				{
					// otherwise, just deactivate the channel
					ch->active = false;
				}
			}
		}
		
		// Volume
		
		ch->volumeCommand = 0; // By default, reset volume command
		
		if (note.volume >= 0x10)
		{
			// something in volume column...
			
			if (note.volume >= 0x10 && note.volume <= 0x50)
			{
				// set volume
				ch->volume = (u8)(note.volume - 0x10);
			}
			else if (note.volume >= 0xC0 && note.volume <= 0xCF)
			{
				// set pan
				ch->pan = (u8)((note.volume-0xC0) << 4);
			}
			else
			{
				ch->volumeCommand = note.volume;
			}
		}
		
		// Command / Parameter
		
		ch->command   = note.command;
		ch->parameter = note.parameter;
		
		ch->noteDelay = 0; // force note delay to zero
	}
	// end processing note
}


void XMPlayer::processEnvelope(XM_Channel* ch, XM_EnvelopeProcess* p_envProcess, XMEnvelope* p_envelope)
{
	if ((p_envelope->flags & XMEnvelope::Flag_Enabled) == 0)
	{
		return;
	}
	if (p_envProcess->done)
	{
		// Envelope is finished
		return;
	}
	
	s16 envLen = (s16)(p_envelope->flags & XMEnvelope::Flag_PointCountMask);
	
	// compute envelope first
	
	if ((p_envelope->flags & XMEnvelope::Flag_SustainEnabled) != 0 &&
	    ch->noteOff == false )
	{
		// if sustain looping
		if (p_envProcess->currentPoint >= p_envelope->sustainIndex )
		{
			p_envProcess->currentPoint = p_envelope->sustainIndex;
			p_envProcess->tick         = 0;
		}
	}
	else if ((p_envelope->flags & XMEnvelope::Flag_LoopEnabled) != 0)
	{
		// else if loop enabled
		
		if (p_envProcess->currentPoint >= p_envelope->loopEndIndex)
		{
			p_envProcess->currentPoint = p_envelope->loopBeginIndex;
			p_envProcess->tick         = 0;
		}
	}
	
	if (p_envProcess->currentPoint >= (envLen - 1) && ( p_envProcess->tick > 0))
	{
		// envelope is terminated. note tick > 0 instead of == 0, as a clever
		// trick to know for certain when the envelope ended
		
		p_envProcess->done         = true;
		p_envProcess->currentPoint = (u8)(envLen - 1);
		if (envLen == 0)
		{
			// a bug, don't bother with it
			return;
		}
	}
	
	// process the envelope
	
	s16 nodeLen = (s16)((p_envProcess->currentPoint >= (envLen-1)) ?
	               0 :
	               (XMEnvelope::getOffset(p_envelope->points[p_envProcess->currentPoint + 1]) -
	               XMEnvelope::getOffset(p_envelope->points[p_envProcess->currentPoint])));
	
	if (nodeLen == 0 || ( p_envProcess->tick == 0))
	{
		// don't interpolate
		p_envProcess->value = (u8)(XMEnvelope::getValue(p_envelope->points[p_envProcess->currentPoint]));
	}
	else
	{
		s16 v1 = (s16)XMEnvelope::getValue(p_envelope->points[p_envProcess->currentPoint]);
		s16 v2 = (s16)XMEnvelope::getValue(p_envelope->points[p_envProcess->currentPoint + 1]);
		s16 r  = (s16)(v1 + p_envProcess->tick * (v2 - v1) / nodeLen);
		
		p_envProcess->value = (u8)r;
	}
	
	// increment
	if (nodeLen != 0)
	{
		p_envProcess->tick++;
		if (p_envProcess->tick >= nodeLen)
		{
			p_envProcess->tick = 0;
			p_envProcess->currentPoint++;
		}
	}
}


// EFFECT COMMAND LISTINGS

enum XM_FX_List
{
	// list must be contiguous, for most compilers to 
	//   create a jump table in the switch()
	XM_FX_0_ARPEGGIO,
	XM_FX_1_PORTA_UP,
	XM_FX_2_PORTA_DOWN,
	XM_FX_3_TONE_PORTA,
	XM_FX_4_VIBRATO,
	XM_FX_5_TONE_PORTA_AND_VOL_SLIDE,
	XM_FX_6_VIBRATO_AND_VOL_SLIDE,
	XM_FX_7_TREMOLO,
	XM_FX_8_SET_PANNING,
	XM_FX_9_SAMPLE_OFFSET,
	XM_FX_A_VOLUME_SLIDE,
	XM_FX_B_POSITION_JUMP,
	XM_FX_C_SET_VOLUME,
	XM_FX_D_PATTERN_BREAK,
	XM_FX_E_SPECIAL,
	XM_FX_F_SET_SPEED_AND_TEMPO,
	XM_FX_G_SET_GLOBAL_VOLUME,
	XM_FX_H_GLOBAL_VOLUME_SLIDE,
	XM_FX_I_UNUSED,
	XM_FX_J_UNUSED,
	XM_FX_K_KEY_OFF,
	XM_FX_L_SET_ENVELOPE_POS,
	XM_FX_M_UNUSED,
	XM_FX_N_UNUSED,
	XM_FX_O_UNUSED,
	XM_FX_P_PAN_SLIDE,
	XM_FX_R_RETRIG,
	XM_FX_S_UNUSED,
	XM_FX_T_TREMOR,
	XM_FX_U_UNUSED,
	XM_FX_V_UNUSED,
	XM_FX_W_UNUSED,
	XM_FX_X_EXTRA_FINE_PORTA,
	XM_FX_Y_UNUSED,
	XM_FX_Z_FILTER // for mixers that can do filtering?
};


enum XM_FX_E_List
{
	XM_FX_E1_FINE_PORTA_UP       = 0x01,
	XM_FX_E2_FINE_PORTA_DOWN     = 0x02,
	XM_FX_E3_SET_GLISS_CONTROL   = 0x03,
	XM_FX_E4_SET_VIBRATO_CONTROL = 0x04,
	XM_FX_E5_SET_FINETUNE        = 0x05,
	XM_FX_E6_SET_LOOP_BEGIN      = 0x06,
	XM_FX_E7_SET_TREMOLO_CONTROL = 0x07,
	XM_FX_E8_UNUSED              = 0x08,
	XM_FX_E9_RETRIG_NOTE         = 0x09,
	XM_FX_EA_FINE_VOL_SLIDE_UP   = 0x0A,
	XM_FX_EB_FINE_VOL_SLIDE_DOWN = 0x0B,
	XM_FX_EC_NOTE_CUT            = 0x0C,
	XM_FX_ED_NOTE_DELAY          = 0x0D,
	XM_FX_EE_PATTERN_DELAY       = 0x0E
};


void XMPlayer::doVibrato(XM_Channel* p_ch)
{
	// 0 .. 32 index, both pos and neg
	u8  idx      = (u8)(_XM_ABS(p_ch->fx4VibratoPhase >> 2) & 0x1F);
	s16 modifier = 0;
	
	switch (p_ch->fx4VibratoType)
	{
	case 0:
		{
			// Sine Wave
			modifier = ms_fx4and7table[idx];
		}
		break;
		
	case 1:
		{
			// Square Wave
			modifier = 0xFF;
		}
		break;
		
	case 2:
		{
			// Saw Wave
			modifier = (s16)(idx << 3);
			if (p_ch->fx4VibratoPhase < 0)
			{
				modifier = (s16)(0xFF - modifier);
			}
		}
		break;
		
	case 3:
		{
			// can't figure this out
		}
		break;
	}
	
	modifier  *= p_ch->fx4VibratoDepth;
	modifier >>= 5;
	
	p_ch->tickrelPeriod += (p_ch->fx4VibratoPhase < 0) ? -modifier : modifier;
	
	if (m_tickCounter > 0)
	{
		p_ch->fx4VibratoPhase += p_ch->fx4VibratoSpeed;
	}
}


void XMPlayer::doTonePorta(XM_Channel* p_ch)
{
	if (p_ch->portaPeriod != 0 && m_tickCounter)
	{
		// porta period must be valid
		
		s32 dist = (s32)p_ch->period - (s32)p_ch->portaPeriod;
		
		if (dist == 0)
		{
			return; // nothing to do, we're at same period
		}
		
		if (_XM_ABS(dist) < (p_ch->fx3Memory << 2))
		{
			// make it reach
			p_ch->period = p_ch->portaPeriod;
		}
		else if (dist < 0)
		{
			// make it raise
			p_ch->period += p_ch->fx3Memory << 2;
		}
		else if (dist > 0)
		{
			p_ch->period -= p_ch->fx3Memory << 2;
		}
	}
}


void XMPlayer::doVolumeSlide(XM_Channel* p_ch)
{
	u8 param     = (p_ch->parameter > 0) ? p_ch->parameter : p_ch->fxAMemory;
	u8 paramUp   = (u8)(param >> 4);
	u8 paramDown = (u8)(param & 0xF);
	
	if (m_tickCounter == 0)
	{
		return;
	}
	
	if (p_ch->parameter != 0)
	{
		p_ch->fxAMemory = p_ch->parameter;
	}
	
	if (paramUp != 0 && paramDown == 0)
	{
		s8 newVol = (s8)(p_ch->volume + paramUp);
		if (newVol > 64)
		{
			newVol = 64;
		}
		p_ch->volume = (u8)newVol;
	}
	else if (paramDown != 0 && paramUp == 0)
	{
		s8 newVol = (s8)(p_ch->volume - paramDown);
		if (newVol < 0)
		{
			newVol = 0;
		}
		p_ch->volume = (u8)newVol;
	}
}


void XMPlayer::processEffectsAndEnvelopes()
{
	int channels = (m_song->flags & XMSong::Flag_ChannelsUsedMask) + 1;
	
	for (int i = 0; i < channels; ++i)
	{
		XM_Channel* ch = &m_channel[i];
		
		// reset pre-effect variables
		
		ch->tickrelPeriod = 0;
		ch->tickrelVolume = 0;
		
		// PROCESS VOLUME COLUMN FOR EFFECT COMMANDS
		
		
		switch ((ch->volumeCommand & 0xF0) >> 4)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			// set volume
			break;
			
		case 6:
			// volume slide down
			{
				if (m_tickCounter == 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfx6Memory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfx6Memory = param;
				}
				
				s8 newVol = (s8)(ch->volume - param);
				if (newVol < 0)
				{
					newVol = 0;
				}
				
				ch->volume = (u8)newVol;
			}
			break;
			
		case 7:
			// volume slide up
			{
				if (m_tickCounter == 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfx7Memory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfx7Memory = param;
				}
				
				s8 newVol = (s8)(ch->volume + param);
				if (newVol > 64)
				{
					newVol = 64;
				}
				
				ch->volume = (u8)newVol;
			}
			break;
			
		case 8:
			// fine volume slide down
			{
				if (m_tickCounter != 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfx8Memory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfx8Memory = param;
				}
				
				s8 newVol = (s8)(ch->volume - param);
				if (newVol < 0)
				{
					newVol = 0;
				}
				
				ch->volume = (u8)newVol;
			}
			break;
			
		case 9:
			// fine volume slide up
			{
				if (m_tickCounter != 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfx9Memory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfx9Memory = param;
				}
				
				s8 newVol = (s8)(ch->volume + param);
				if (newVol > 64)
				{
					newVol = 64;
				}
				
				ch->volume = (u8)newVol;
			}
			break;
			
		case 0xA:
			// set vibrato speed
			{
				if (m_tickCounter == 0)
				{
					ch->volfxAVibratoSpeed = u8((ch->volumeCommand & 0x0F) << 2);
				}
			}
			break;
			
		case 0xB:
			// vibrato
			{
				if (m_tickCounter == 0)
				{
					if (ch->rowHasNote)
					{
						ch->volfxBVibratoPhase = 0;
					}
					
					if ((ch->volumeCommand & 0x0F) != 0)
					{
						ch->volfxBVibratoDepth = (u8)(ch->volumeCommand & 0x0F);
					}
				}
				// do vibrato
				// 0 .. 32 index, both pos and neg
				u8  idx      = (u8)(_XM_ABS(ch->volfxBVibratoPhase >> 2) & 0x1F);
				s16 modifier = 0;
				
				switch (ch->fx4VibratoType)
				{
				case 0:
					{
						// Sine Wave
						modifier = ms_fx4and7table[idx];
					}
					break;
					
				case 1:
					{
						// Square Wave
						modifier = 0xFF;
					}
					break;
					
				case 2:
					{
						// Saw Wave
						modifier = (s16)(idx << 3);
						if (ch->volfxBVibratoPhase < 0)
						{
							modifier = (s16)(0xFF - modifier);
						}
					}
					break;
					
				case 3:
					{
						// can't figure this out
					}
					break;
				}
				
				modifier  *= ch->volfxBVibratoDepth;
				modifier >>= 5;
				
				ch->tickrelPeriod += (ch->volfxBVibratoPhase < 0) ? -modifier : modifier;
				
				if (m_tickCounter > 0)
				{
					ch->volfxBVibratoPhase += ch->volfxAVibratoSpeed;
				}
			}
			break;
			
		case 0xC:
			// set panning
			{
				if (m_tickCounter == 0)
				{
					ch->pan = u8((ch->volumeCommand & 0x0F) << 4);
				}
			}
			break;
			
		case 0xD:
			// panning slide left
			{
				if (m_tickCounter == 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfxDMemory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfxDMemory = param;
				}
				
				s16 newPan = (s16)(ch->pan - param);
				if (newPan < 0)
				{
					newPan = 0;
				}
				
				ch->pan = (u8)newPan;
			}
			break;
			
		case 0xE:
			// panning slide right
			{
				if (m_tickCounter == 0)
				{
					break;
				}
				
				u8 param = ((ch->volumeCommand & 0x0F) > 0) ? u8(ch->volumeCommand & 0x0F) : ch->volfxEMemory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfxEMemory = param;
				}
				
				s16 newPan = (s16)(ch->pan + param);
				if (newPan > 255)
				{
					newPan = 255;
				}
				
				ch->pan = (u8)newPan;
			}
			break;
			
		case 0xF:
			// Tone porta
			{
				u8 param = (ch->volumeCommand & 0x0F) ? u8((ch->volumeCommand & 0x0F) << 4) : ch->volfxFMemory;
				if ((ch->volumeCommand & 0x0F) != 0)
				{
					ch->volfxFMemory = param;
				}
				
				if (m_tickCounter == 0 && ch->rowHasNote)
				{
					ch->portaPeriod = ch->period;
					ch->period      = ch->oldPeriod;
					ch->restart     = false;
				}
				
				// do Tone Porta
				if (ch->portaPeriod != 0 && m_tickCounter != 0)
				{
					// porta period must be valid
					
					s32 dist = (s32)ch->period - (s32)ch->portaPeriod;
					
					if (dist == 0)
					{
						break; // nothing to do, we're at same period
					}
					
					if (_XM_ABS(dist) < (ch->volfxFMemory << 2))
					{
						// make it reach
						ch->period = ch->portaPeriod;
					}
					else if (dist < 0)
					{
						// make it raise
						ch->period += ch->volfxFMemory << 2;
					}
					else if (dist > 0)
					{
						ch->period -= ch->volfxFMemory << 2;
					}
				}
			}
			break;
		}
		
		
		// PROCESS  EFFECT COMMANDS
		switch (ch->command)
		{
		case XM_FX_0_ARPEGGIO:
			{
				s32 basePeriod = static_cast<s32>(getPeriod(ch->realNote, ch->finetune));
				u8 ofs         = 0; 
				
				switch (m_tickCounter % 3)
				{
					case 0: break;
					case 1: ofs = (u8)(ch->parameter >> 4);  break;
					case 2: ofs = (u8)(ch->parameter & 0xF); break;
				}
				
				ch->tickrelPeriod += (s32)getPeriod((s16)(ch->realNote + ofs),
				                                    ch->finetune) - basePeriod;
			}
			break;
			
		case XM_FX_1_PORTA_UP:
			{
				if (m_tickCounter == 0)
				{
					break;
				}
				
				u8 param = (ch->parameter > 0) ? ch->parameter : ch->fx1Memory;
				if (ch->parameter != 0)
				{
					ch->fx1Memory = param;
				}
				ch->period -= param << 2;
			}
			break;
			
		case XM_FX_2_PORTA_DOWN:
			{
				u8 param;
				if (m_tickCounter == 0)
				{
					break;
				}
				
				param = (ch->parameter > 0) ? ch->parameter : ch->fx2Memory;
				if (ch->parameter != 0)
				{
					ch->fx2Memory = ch->parameter;
				}
				
				ch->period += param << 2;
			}
			break;
			
		case XM_FX_3_TONE_PORTA:
			{
				u8 param = (ch->parameter > 0) ? ch->parameter : ch->fx3Memory;
				if (ch->parameter != 0)
				{
					ch->fx3Memory = param;
				}
				
				if (m_tickCounter == 0 && ch->rowHasNote)
				{
					ch->portaPeriod = ch->period;
					ch->period      = ch->oldPeriod;
					ch->restart     = false;
				}
				
				doTonePorta(ch);
			}
			break;
			
		case XM_FX_4_VIBRATO:
			{
				// reset phase on new note
				if (m_tickCounter == 0)
				{
					if (ch->rowHasNote)
					{
						ch->fx4VibratoPhase = 0;
					}
					
					if ((ch->parameter & 0xF) != 0)
					{
						ch->fx4VibratoDepth = (u8)(ch->parameter & 0xF);
					}
					
					if ((ch->parameter & 0xF0) != 0)
					{
						ch->fx4VibratoSpeed = (u8)((ch->parameter & 0xF0) >> 2);
					}
				}
				
				doVibrato(ch);
			}
			break;
			
		case XM_FX_5_TONE_PORTA_AND_VOL_SLIDE:
			{
				doVolumeSlide(ch);
				doTonePorta(ch);
			}
			break;
			
		case XM_FX_6_VIBRATO_AND_VOL_SLIDE:
			{
				doVolumeSlide(ch);
				doVibrato(ch);
			}
			break;
			
		case XM_FX_7_TREMOLO:
			{
				// 0 .. 32 index, both pos and neg
				u8 idx       = (u8)(_XM_ABS(ch->fx7TremoloPhase >> 2) & 0x1F);
				s16 modifier = 0;
				
				switch (ch->fx7TremoloType)
				{
				case 0:
					{
						// Sine Wave
						modifier = ms_fx4and7table[idx];
					}
					break;
					
				case 1:
					{
						// Square Wave
						modifier = 0xFF;
					}
					break;
					
				case 2:
					{
						// Saw Wave
						modifier = (s16)(idx << 3);
						if (ch->fx7TremoloPhase < 0)
						{
							modifier = (s16)(0xFF - modifier);
						}
					}
					break;
					
				case 3:
					{
						// can't figure this out
					}
					break;
				}
				
				modifier  *= ch->fx7TremoloDepth;
				modifier >>= 7;
				
				ch->tickrelVolume += (ch->fx7TremoloPhase < 0) ? -modifier : modifier;
				
				if (m_tickCounter > 0)
				{
					ch->fx7TremoloPhase += ch->fx7TremoloSpeed;
				}
				
			}
			break;
			
		case XM_FX_8_SET_PANNING:
			{
				if (m_tickCounter == 0)
				{
					ch->pan = ch->parameter;
				}
			}
			break;
			
		case XM_FX_9_SAMPLE_OFFSET:
			{
				ch->restartOffset = (u32)ch->parameter << 8;
			}
			break;
			
		case XM_FX_A_VOLUME_SLIDE:
			{
				doVolumeSlide(ch);
			}
			break;
			
		case XM_FX_B_POSITION_JUMP:
			{
				if (m_tickCounter != 0 || ch->parameter >= m_song->orderCount)
				{
					// pointless
					break;
				}
				
				if (m_forceNextOrder)
				{
					// already forced? just change order
					m_forcedNextOrder = ch->parameter;
				}
				else
				{
					m_forceNextOrder  = true;
					m_forcedNextOrder = ch->parameter;
				}
			} break;
			
		case XM_FX_C_SET_VOLUME:
			{
				if (m_tickCounter != 0 || ch->parameter > 64)
				{
					break;
				}
				
				ch->volume = ch->parameter;
			}
			break;
			
		case XM_FX_D_PATTERN_BREAK:
			{
				if (m_tickCounter != 0)
				{
					// pointless
					break;
				}
				
				if (m_forceNextOrder)
				{
					// already forced? just change order
					m_forcedNextOrderRow = ch->parameter;
				}
				else
				{
					m_forceNextOrder     = true;
					m_forcedNextOrderRow = ch->parameter;
					m_forcedNextOrder    = (u8)(m_currentOrder + 1);
				}
			}
			break;
			
		case XM_FX_E_SPECIAL:
			{
				u8 ecmd   = (u8)(ch->parameter >> 4);
				u8 eparam = (u8)(ch->parameter & 0xF);
				switch (ecmd)
				{
				case XM_FX_E1_FINE_PORTA_UP:
					{
						u8 param;
						if (m_tickCounter != 0)
						{
							break;
						}
						param = (eparam > 0) ? eparam : ch->fxE1Memory;
						if (eparam != 0)
						{
							ch->fxE2Memory = param;
						}
						ch->period -= param << 2;
					}
					break;
					
				case XM_FX_E2_FINE_PORTA_DOWN:
					{
						u8 param;
						if (m_tickCounter != 0)
						{
							break;
						}
						param = (eparam > 0) ? eparam : ch->fxE2Memory;
						if (eparam != 0)
						{
							ch->fxE2Memory = param;
						}
						ch->period += param << 2;
					}
					break;
					
				case XM_FX_E3_SET_GLISS_CONTROL:
					{
						// IGNORED, DEPRECATED
					}
					break;
					
				case XM_FX_E4_SET_VIBRATO_CONTROL:
					{
						if (m_tickCounter != 0)
						{
							break;
						}
						
						if (eparam < 4)
						{
							ch->fx4VibratoType = eparam;
						}
					}
					break;
					
				case XM_FX_E5_SET_FINETUNE:
					{
						if (eparam < 4)
						{
							ch->finetune = (s8)(((s8)eparam - 8) << 4);
						}
					}
					break;
					
				case XM_FX_E6_SET_LOOP_BEGIN:
					{
						// IGNORED, difficult to
						// support in hardware
					}
					break;
					
				case XM_FX_E7_SET_TREMOLO_CONTROL:
					{
						if (m_tickCounter != 0)
						{
							break;
						}
						
						if (eparam < 4)
						{
							ch->fx7TremoloType = eparam;
						}
					}
					break;
					
				case XM_FX_E8_UNUSED:
					break;
					
				case XM_FX_E9_RETRIG_NOTE:
					{
						// this needs more testing... it gets a lot of validations already
						
						if (eparam != 0 && 
						    (m_tickCounter % eparam) == 0 &&
						    ch->oldPeriod != 0 &&
						    ch->samplePtr != 0 &&
						    ch->instrumentPtr != 0 )
						{
							ch->restart = true;
							envelopeReset(&ch->volumeEnvelope);
							envelopeReset(&ch->panEnvelope);
							ch->active = true;
						}
					}
					break;
					
				case XM_FX_EA_FINE_VOL_SLIDE_UP:
					{
						if (m_tickCounter != 0)
						{
							break;
						}
						
						u8 param = (eparam > 0) ? eparam : ch->fxEAMemory;
						if (eparam != 0)
						{
							ch->fxEAMemory = param;
						}
						
						s8 newVol = (s8)(ch->volume + param);
						if (newVol > 64)
						{
							newVol = 64;
						}
						
						ch->volume = (u8)newVol;
					}
					break;
					
				case XM_FX_EB_FINE_VOL_SLIDE_DOWN:
					{
						if (m_tickCounter != 0)
						{
							break;
						}
						
						u8 param = (eparam > 0) ? eparam : ch->fxEBMemory;
						if (eparam != 0)
						{
							ch->fxEBMemory = param;
						}
						
						s8 newVol = (s8)(ch->volume - param);
						if (newVol < 0)
						{
							newVol = 0;
						}
						
						ch->volume = (u8)newVol;
					}
					break;
					
				case XM_FX_EC_NOTE_CUT:
					{
						if (m_tickCounter == eparam)
						{
							ch->active = false;
						}
					}
					break;
					
				case XM_FX_ED_NOTE_DELAY:
					{
						if (m_tickCounter != 0 || ch->noteDelay >= m_currentSpeed)
						{
							break;
						}
						
						ch->noteDelay = eparam;
					}
					break;
					
				case XM_FX_EE_PATTERN_DELAY:
					{
						if (m_tickCounter != 0)
						{
							break;
						}
						
						m_patternDelay = eparam;
					}
					break;
				}
			}
			break;
			
		case XM_FX_F_SET_SPEED_AND_TEMPO:
			{
				if (m_tickCounter != 0)
				{
					// pointless
					break;
				}
				
				if (ch->parameter == 0)
				{
					break;
				}
				else if (ch->parameter < 0x1F)
				{
					m_currentSpeed = ch->parameter;
				}
				else
				{
					m_currentTempo = ch->parameter;
				}
			}
			break;
			
		case XM_FX_G_SET_GLOBAL_VOLUME:
			{
				if (m_tickCounter != 0 || ch->parameter > 64)
				{
					// pointless
					break;
				}
				m_globalVolume = ch->parameter;
			}
			break;
			
		case XM_FX_H_GLOBAL_VOLUME_SLIDE:
			{
				if (m_tickCounter == 0)
				{
					return;
				}
				
				u8 param     = (ch->parameter > 0) ? ch->parameter : ch->fxHMemory;
				u8 paramUp   = (u8)(param >> 4);
				u8 paramDown = (u8)(param & 0xF);
				
				if (ch->parameter != 0)
				{
					ch->fxHMemory = ch->parameter;
				}
				
				if (paramUp != 0)
				{
					s8 newVol = (s8)(m_globalVolume + paramUp);
					if (newVol > 64)
					{
						newVol = 64;
					}
					m_globalVolume = (u8)newVol;
				}
				else if (paramDown != 0)
				{
					// up has priority over down
					
					s8 newVol = (s8)(m_globalVolume - paramDown);
					if (newVol < 0)
					{
						newVol = 0;
					}
					m_globalVolume = (u8)newVol;
				}
			}
			break;
			
		case XM_FX_I_UNUSED: break;
		case XM_FX_J_UNUSED: break;
			
		case XM_FX_K_KEY_OFF:
			{
				ch->noteOff = true;
			}
			break;
			
		case XM_FX_L_SET_ENVELOPE_POS:
			{
				// this is weird, should i support it? Impulse Tracker doesn't..
			}
			break;
			
		case XM_FX_M_UNUSED: break;
		case XM_FX_N_UNUSED: break;
		case XM_FX_O_UNUSED: break;
			
		case XM_FX_P_PAN_SLIDE:
			{
				if (m_tickCounter == 0)
				{
					return;
				}
				
				u8 param     = (ch->parameter > 0) ? ch->parameter : ch->fxPMemory;
				u8 paramUp   = (u8)(param >> 4);
				u8 paramDown = (u8)(param & 0xF);
				
				if (ch->parameter != 0)
				{
					ch->fxPMemory = ch->parameter;
				}
				
				if (paramUp != 0)
				{
					s16 newPan = (s16)(ch->pan + paramUp);
					if (newPan > 255)
					{
						newPan = 255;
					}
					
					ch->pan = (u8)newPan;
				}
				else if (paramDown != 0)
				{
					// up has priority over down
					
					s16 newPan = (s16)(ch->pan - paramDown);
					if (newPan < 0)
					{
						newPan = 0;
					}
					
					ch->pan = (u8)newPan;
				}
			}
			break;
			
		case XM_FX_R_RETRIG:
			{
				
			}
			break;
			
		case XM_FX_S_UNUSED: break;
			
		case XM_FX_T_TREMOR:
			{
				
			}
			break;
			
		case XM_FX_U_UNUSED: break;
		case XM_FX_V_UNUSED: break;
		case XM_FX_W_UNUSED: break;
			
		case XM_FX_X_EXTRA_FINE_PORTA:
			{
				
			}
			break;
			
		case XM_FX_Y_UNUSED: break;
			
		case XM_FX_Z_FILTER:
			{
				
			}
			break;
		}
		
		// avoid zero or negative period
		
		if (ch->period <= 0)
		{
			ch->active = false;
		}
		
		// process note delay
		
		if (ch->noteDelay != 0)
		{
			continue;
		}
		
		if (ch->active)
		{
			processEnvelope(ch, &ch->volumeEnvelope, &ch->instrumentPtr->volumeEnvelope);
			processEnvelope(ch, &ch->panEnvelope,    &ch->instrumentPtr->panEnvelope);
		}
	}
}


void XMPlayer::updateMixer()
{
	int channels         = (m_song->flags & XMSong::Flag_ChannelsUsedMask) + 1;
	int mixerMaxChannels = (int)(m_mixer->getRestrictions() & XMMixer::Restriction_MaxVoicesMask);
	
	for (int i = 0; i < channels; ++i )
	{
		XM_Channel* ch = &m_channel[i];
		
		if (ch->active == false)
		{
			if (m_mixer->voiceIsActive((u8)i))
			{
				m_mixer->voiceStop((u8)i);
			}
			continue;
		}
		
		// reset pre-effect variables
		if (i >= mixerMaxChannels)
		{
			// channel unsupported
			continue;
		}
		
		if (ch->noteDelay != 0)
		{
			// if requested delay, don't do a thing
			ch->noteDelay--;
			continue;
		}
		
		// start voice if needed
		if (ch->restart)
		{
			m_mixer->voiceStart((u8)i, ch->samplePtr->sampleId, ch->restartOffset);
			ch->restart = false;
		}
		
		// check channel activity
		if (ch->active && m_mixer->voiceIsActive((u8)i) == false)
		{
			ch->active = false;
			continue;
		}
		
		if (ch->active == false && m_mixer->voiceIsActive((u8)i))
		{
			m_mixer->voiceStop((u8)i);
			continue;
		}
		
		// VOLUME PROCESS
		s16 finalVolume;
		
		finalVolume = (s16)(((ch->volume + ch->tickrelVolume) * 255) >> 6);
		
		if ((ch->instrumentPtr->volumeEnvelope.flags & XMEnvelope::Flag_Enabled) != 0)
		{
			finalVolume = (s16)((finalVolume * ch->volumeEnvelope.value) >> 6);
			
			if (ch->noteOff)
			{
				u16 fadeDown = ch->instrumentPtr->fadeout;
				
				if (fadeDown > 0xFFF || fadeDown >= ch->fadeout)
				{
					ch->fadeout = 0;
				}
				else
				{
					ch->fadeout -= fadeDown;
				}
				
				if (ch->fadeout == 0)
				{
					ch->active = false;
				}
			}
		}
		
		finalVolume = (s16)((finalVolume * m_globalVolume ) >> 6);
		
		if (finalVolume > 255)
		{
			finalVolume = 255;
		}
		
		if (finalVolume < 0)
		{
			finalVolume = 0;
		}
		
		m_mixer->voiceSetVolume((u8)i, (u8)finalVolume);
		
		// PAN PROCESS
		s16 finalPan = ch->pan;
		
		if ((ch->instrumentPtr->panEnvelope.flags & XMEnvelope::Flag_Enabled) != 0)
		{
			finalPan = (s16)(finalPan + ((s16)ch->panEnvelope.value - 32) * (128 - _XM_ABS(finalPan - 128)) / 32);
		}
		
		if (finalPan < 0)
		{
			finalPan = 0;
		}
		if (finalPan > 255)
		{
			finalPan = 255;
		}
		
		m_mixer->voiceSetPan((u8)i, (u8)finalPan);
		
		m_mixer->voiceSetSpeed((u8)i, getFrequency((u32)(ch->period + ch->tickrelPeriod)) );
	}
	
	// re convert tempo to usecs
	m_mixer->setProcessCallbackInterval((u32)(2500000 / m_currentTempo));
}


bool XMPlayer::processTick()
{
	if (m_song == 0)
	{
		// if song set is null, don't do a thing
		return false; 
	}
	
	if (m_active == false)
	{
		return false;
	}
	
	bool cont = true;
	
	// Check Ticks
	
	if (m_tickCounter >= (m_currentSpeed + m_patternDelay))
	{
		// Tick Reaches Zero, process row
		
		m_tickCounter  = 0;
		m_patternDelay = 0;
		
		// change order, as requested by some command
		if (m_forceNextOrder)
		{
			if (m_forcedNextOrder < m_song->orderCount)
			{
				m_currentOrder   = m_forcedNextOrder;
				m_currentRow     = m_forcedNextOrderRow;
				m_currentPattern = m_song->orderList[m_currentOrder];
			}
			
			m_forceNextOrder     = false;
			m_forcedNextOrderRow = 0;
		}
		
		if (m_forceNextPattern)
		{
			m_currentPattern = m_forcedNextPattern;
			m_currentRow     = 0;
			
			m_forceNextPattern  = false;
			m_forcedNextPattern = 0;
		}
		
		// process a row of notes
		processNotes();
		
		// increment row and check pattern/order changes
		m_currentRow++;
		
		if (m_currentRow >= getPatternLength((u8)m_currentPattern))
		{
			m_currentRow = 0;
			m_currentOrder++;
			
			if (m_currentOrder >= m_song->orderCount)
			{
				m_reachedEnd = true;
				m_currentOrder = m_song->restartPos;
				if (m_looping == false)
				{
					cont = false;
				}
			}
			
			m_currentPattern = m_song->orderList[m_currentOrder];
		}
	}
	
	// PROCESS EFFECTS AND ENVELOPES
	
	processEffectsAndEnvelopes();
	
	// UPDATE MIXER
	
	updateMixer();
	
	// DECREMENT TICK
	m_tickCounter++;
	
	return cont;
}


void XMPlayer::resetTempo()
{
	m_currentTempo = m_song->tempo;
}


void XMPlayer::forceNextPattern(u8 p_pattern)
{
	m_forceNextPattern  = true;
	m_forcedNextPattern = p_pattern;
}


void XMPlayer::forceNextOrder(u8 p_order)
{
	m_forceNextOrder  = true;
	m_forcedNextOrder = p_order;
}


void XMPlayer::setSong(XMSong* p_song)
{
	if (m_active)
	{
		stop();
	}
	
	m_song = p_song;
	if (m_song != 0)
	{
		m_song->setPlayer(this);
	}
	reset();
}


void XMPlayer::play(bool p_looping, u8 p_startOrder)
{
	if (m_song == 0)
	{
		TT_PANIC("NO SONG SET IN PLAYER");
		return;
	}
	
	stop(); // stop if playing
	
	//reset(); // Reset is already done in stop
	m_currentOrder = p_startOrder;
	if (m_song != 0)
	{
		m_currentPattern = m_song->orderList[m_currentOrder];
	}
	m_active = true;
	
	m_mixer->play();
	m_looping = p_looping;
}


void XMPlayer::stop()
{
	if (m_mixer != 0)
	{
		m_mixer->resetVoices();
		m_mixer->stop();
	}
	
	reset();
	m_active = false;
}


void XMPlayer::setMixer(XMMixer *p_mixer)
{
	if (XMUtil::getMemoryManager() == 0)
	{
		TT_PANIC("MEMORY MANAGER NOT CONFIGURED");
		return;
	}
	
	if (m_mixer != 0)
	{
		TT_PANIC("XM MIXER ALREADY CONFIGURED");
		return;
	}
	
	m_mixer = p_mixer;
	m_mixer->setPlayer(this);
}

// Namespace end
}
}
}
