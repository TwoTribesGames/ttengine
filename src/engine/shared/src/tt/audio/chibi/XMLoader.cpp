#include <tt/audio/chibi/XMEnvelope.h>
#include <tt/audio/chibi/XMFileIO.h>
#include <tt/audio/chibi/XMInstrument.h>
#include <tt/audio/chibi/XMLoader.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMMixer.h>
#include <tt/audio/chibi/XMPlayer.h>
#include <tt/audio/chibi/XMSong.h>
#include <tt/audio/codec/ttadpcm/ImaAdpcm.h>
#include <tt/code/bufferutils.h>
#include <tt/code/FourCC.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#ifdef XM_DEBUG
	#define XM_Printf TT_Printf
#else
	#define XM_Printf(...)
#endif


namespace tt {
namespace audio {
namespace chibi {

XMLoader::XMLoader()
:
m_fileIO(0),
m_player(0)
{
}


XMLoader::Error XMLoader::openSong(const char* p_fileName, XMSong* p_song)
{
	return openSongCustom(p_fileName, p_song, true, true);
}


XMLoader::Error XMLoader::openMusic(const char* p_fileName, XMSong* p_song)
{
	return openSongCustom(p_fileName, p_song, true, false);
}


XMLoader::Error XMLoader::openInstruments(const char* p_fileName, XMSong* p_song)
{
	return openSongCustom(p_fileName, p_song, false, true);
}


XMLoader::Error XMLoader::openCustomMusic(const char* p_fileName, XMSong* p_song)
{
	if (m_fileIO                   == 0 ||
	    m_player                   == 0 ||
	    m_player->getMixer()       == 0 ||
	    XMUtil::getMemoryManager() == 0)
	{
		// Check whether we have everything needed to go
		TT_WARNING(m_fileIO != 0, "MISSING FILEIO");
		TT_WARNING(m_player != 0, "MISSING PLAYER");
		TT_WARNING(m_player != 0 && m_player->getMixer() != 0, "MISSING MIXER");
		TT_WARNING(XMUtil::getMemoryManager() != 0, "MISSING MEMORY MANAGER");
		
		return Error_Unconfigured;
	}
	
	if (m_fileIO->inUse())
	{
		return Error_FileIOInUse;
	}
		
	if (m_fileIO->open(p_fileName, false) == XMFileIO::IOError_CantOpen)
	{
		return Error_FileCantOpen;
	}
	
	// read signature
	const u32 signature = m_fileIO->getU32();
	if (signature == code::FourCC<'C', 'X', 'M', 'P'>::value)
	{
		m_fileIO->decompress();
	}
	else if (signature != code::FourCC<'U', 'X', 'M', 'P'>::value)
	{
		TT_PANIC("Old patterndata, please update your converters and rebuild your data.");
		m_fileIO->close();
		return Error_FileCantOpen;
	}
	
	// Load header
	u16 patterncount;
	{
		p_song->orderCount = (u8)(m_fileIO->getU16());
		
		p_song->restartPos = (u8)(m_fileIO->getU16());
		if (p_song->restartPos >= p_song->orderCount)
		{
			p_song->restartPos = 0;
		}
		
		u8 chans = (u8)(m_fileIO->getU16());
		if (chans > XM_MaxChannels)
		{
			TT_WARN("Invalid Number of Channels: %i > %i", chans, XM_MaxChannels);
			return Error_FileCorrupt;
		}
		
		// use 5 bits, +1
		p_song->flags = (u8)((chans - 1) & 0x1F);
		patterncount  = p_song->patternCount = (u8)m_fileIO->getU16();
		
		// flags. only linear periods
		if (m_fileIO->getU16() != 0)
		{
			p_song->flags |= XMSong::Flag_LinearPeriods;
		}
		
		p_song->speed = m_fileIO->getU16();
		p_song->tempo = m_fileIO->getU16();
		
		m_fileIO->getByteArray(p_song->orderList, 256);
		
		XM_Printf("Song Header:\n");
		XM_Printf("\tChannels: %i\n", chans);
		XM_Printf("\tOrders: %i\n", p_song->orderCount);
		XM_Printf("\tPatterns: %i\n", p_song->patternCount);
		XM_Printf("\tRestart At: %i\n", p_song->restartPos);
		XM_Printf("\tTempo: %i\n", p_song->tempo);
		XM_Printf("\tSpeed: %i\n", p_song->speed);
		XM_Printf("\n");
	}
	
	// Load patterns
	if (p_song->patternCount != 0)
	{
		p_song->patternData = (u8**)XMUtil::getMemoryManager()->alloc(sizeof(u8*)*p_song->patternCount, XMMemoryManager::AllocType_Pattern);
		
		if (p_song->patternData == 0)
		{
			// Handle OUT OF MEMORY
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		XMUtil::getMemoryManager()->zeroMem(p_song->patternData, sizeof(u8*) * p_song->patternCount);
	}
	
	for (int i = 0; i < patterncount; ++i)
	{
		u8  packing        = m_fileIO->getU8();  // if packing = 1, this pattern is pre-packed
		u16 rows           = m_fileIO->getU16(); // rows
		u16 packedDataSize = m_fileIO->getU16(); // pattern header length
		
		XM_Printf("Pattern: %i\n", i);
		XM_Printf("\tRows: %i\n", rows);
		XM_Printf("\tPacked Size: %i\n", packedDataSize);
		
		if (packedDataSize == 0)
		{
			p_song->patternData[i] = 0;
		}
		else if (packing == 1)
		{
			// pre packed pattern
			u8* pdata = (u8*)XMUtil::getMemoryManager()->alloc(packedDataSize, XMMemoryManager::AllocType_Pattern);
			if (pdata == 0)
			{
				// Handle OUT OF MEMORY
				p_song->clear(i, -1);
				m_fileIO->close();
				
				return Error_OutOfMemory;
			}
			
			m_fileIO->getByteArray(pdata, packedDataSize);
			
			p_song->patternData[i] = pdata;
		}
		else
		{
			// TODO: read all sample data from file and recompress using XMCompressor class
			
			// pack on the fly while reading
			u32 packBeginPos = m_fileIO->getPos();
			
			// just calculate size
			u32 repackedSize = recompressPattern(rows, (u8)((p_song->flags & 0x1f) + 1), 0);
			
			XM_Printf("\tRePacked Size: %i\n", repackedSize);
			
			m_fileIO->seekPos(packBeginPos);
			
			u8* pdata = (u8*)XMUtil::getMemoryManager()->alloc(1 + repackedSize, XMMemoryManager::AllocType_Pattern);
			
			if (pdata == 0)
			{
				// Handle OUT OF MEMORY
				p_song->clear(i, -1);
				m_fileIO->close();
				
				return Error_OutOfMemory;
			}
			
			// first byte is rows
			pdata[0] = (u8)(rows - 1);
			
			// on the fly recompress
			recompressPattern(rows, (u8)((p_song->flags & 0x1f) + 1), &pdata[1]);
			
			p_song->patternData[i] = pdata;
		}
	}
	
	m_fileIO->close();
	return Error_Ok;
}


XMLoader::Error XMLoader::openCustomInstruments(const char* p_fileName, XMSong* p_song)
{
	if (m_fileIO                   == 0 ||
	    m_player                   == 0 ||
	    m_player->getMixer()       == 0 ||
	    XMUtil::getMemoryManager() == 0)
	{
		// Check whether we have everything needed to go
		TT_WARNING(m_fileIO != 0, "MISSING FILEIO");
		TT_WARNING(m_player != 0, "MISSING PLAYER");
		TT_WARNING(m_player != 0 && m_player->getMixer() != 0, "MISSING MIXER");
		TT_WARNING(XMUtil::getMemoryManager() != 0, "MISSING MEMORY MANAGER");
		
		return Error_Unconfigured;
	}
	
	if (m_fileIO->inUse())
	{
		return Error_FileIOInUse;
	}
		
	if (m_fileIO->open(p_fileName, false) == XMFileIO::IOError_CantOpen)
	{
		return Error_FileCantOpen;
	}
	
	// read signature
	const u32 signature = m_fileIO->getU32();
	if (signature == code::FourCC<'C', 'X', 'M', 'I'>::value)
	{
		m_fileIO->decompress();
	}
	else if (signature != code::FourCC<'U', 'X', 'M', 'I'>::value)
	{
		TT_PANIC("Old instrument data, please update your converters and rebuild your data.");
		m_fileIO->close();
		return Error_FileCantOpen;
	}
	
	p_song->instrumentCount = (u8)m_fileIO->getU16();
	
	if (p_song->instrumentCount != 0)
	{
		p_song->instrumentData = (XMInstrument**)XMUtil::getMemoryManager()->alloc(sizeof(XMInstrument*) * p_song->instrumentCount, XMMemoryManager::AllocType_Instrument);
		
		if (p_song->instrumentData == 0)
		{
			// Handle OUT OF MEMORY
			p_song->clear(-1, 0);
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		XMUtil::getMemoryManager()->zeroMem(p_song->instrumentData, sizeof(XMInstrument*) * p_song->instrumentCount);
	}
	else
	{
		// Don't load instruments
		m_fileIO->close();
		
		return Error_Ok;
	}
	
	for (int i = 0; i < p_song->instrumentCount; ++i)
	{
		// ignore type
		m_fileIO->getU8();
		
		u16 samples = m_fileIO->getU16();
		
		XM_Printf("Instrument: %i\n", i);
		
		if (samples == 0)
		{
			// empty instrument
			p_song->instrumentData[i] = 0;
			
			XM_Printf("\tSkipped!\n");
			continue;
		}
		else if (samples > XMConstant_MaxSamplesPerInstrument)
		{
			TT_WARN("\tHas invalid sample count: %i\n", samples);
			
			p_song->clear(-1, i);
			m_fileIO->close();
			
			return Error_FileCorrupt;
		}
		else
		{
			p_song->instrumentData[i] = (XMInstrument*)XMUtil::getMemoryManager()->alloc(sizeof(XMInstrument), XMMemoryManager::AllocType_Instrument);
			
			if (p_song->instrumentData[i] == 0)
			{
				// Out of Memory
				p_song->clear(-1, i);
				m_fileIO->close();
				
				return Error_OutOfMemory;
			}
			
			XMUtil::getMemoryManager()->zeroMem(p_song->instrumentData[i], sizeof(XMInstrument));
		}
		
		XMInstrument* instrument = p_song->instrumentData[i];
		instrument->sampleCount  = (u8)samples;
		
		// reset the samples
		instrument->samples = 0;
		
		for (int j = 0; j < 48; ++j)
		{
			// convert to nibbles
			u8 nibble = (u8)(m_fileIO->getU8() & 0xF);
			nibble   |= m_fileIO->getU8() << 4;
			
			instrument->noteSample[j] = nibble;
		}
		
		for (int j = 0; j < 12; ++j)
		{
			u16 ofs = m_fileIO->getU16();
			u16 val = m_fileIO->getU16();
			
			// encode into 16 bits
			instrument->volumeEnvelope.points[j] = (u16)((val << 9) | ofs);
		}
		for (int j = 0; j < 12; ++j)
		{
			u16 ofs = m_fileIO->getU16();
			u16 val = m_fileIO->getU16();
			
			// encode into 16 bits
			instrument->panEnvelope.points[j] = (u16)((val << 9) | ofs);
		}
		
		instrument->volumeEnvelope.flags = m_fileIO->getU8();
		instrument->panEnvelope.flags    = m_fileIO->getU8();
		
		instrument->volumeEnvelope.sustainIndex   = m_fileIO->getU8();
		instrument->volumeEnvelope.loopBeginIndex = m_fileIO->getU8();
		instrument->volumeEnvelope.loopEndIndex   = m_fileIO->getU8();
		
		instrument->panEnvelope.sustainIndex   = m_fileIO->getU8();
		instrument->panEnvelope.loopBeginIndex = m_fileIO->getU8();
		instrument->panEnvelope.loopEndIndex   = m_fileIO->getU8();
		
		{
			// Volume Envelope Flags
			u8 envelopeFlags = m_fileIO->getU8();
			if ((envelopeFlags & 1) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_Enabled;
			}
			if ((envelopeFlags & 2) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_SustainEnabled;
			}
			if ((envelopeFlags & 4) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_LoopEnabled;
			}
		}
		{
			// Panning Envelope Flags
			u8 envelopeFlags = m_fileIO->getU8();
			
			if ((envelopeFlags & 1) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_Enabled;
			}
			if ((envelopeFlags & 2) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_SustainEnabled;
			}
			if ((envelopeFlags & 4) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_LoopEnabled;
			}
		}
		
		instrument->vibratoType  = (XMVibratoType)m_fileIO->getU8();
		instrument->vibratoSweep = m_fileIO->getU8();
		instrument->vibratoDepth = m_fileIO->getU8();
		instrument->vibratoRate  = m_fileIO->getU8();
		instrument->fadeout      = m_fileIO->getU16();
		
		XM_Printf("\tVolume Envelope:\n");
		XM_Printf("\t\tPoints: %i\n",        instrument->volumeEnvelope.flags&XM_ENVELOPE_POINT_COUNT_MASK);
		XM_Printf("\t\tEnabled: %s\n",      (instrument->volumeEnvelope.flags&XM_ENVELOPE_ENABLED) ?         "Yes" : "No");
		XM_Printf("\t\tSustain: %s\n",      (instrument->volumeEnvelope.flags&XM_ENVELOPE_SUSTAIN_ENABLED) ? "Yes" : "No");
		XM_Printf("\t\tLoop Enabled: %s\n", (instrument->volumeEnvelope.flags&XM_ENVELOPE_LOOP_ENABLED) ?    "Yes" : "No");
		
		XM_Printf("\tPan Envelope:\n");
		XM_Printf("\t\tPoints: %i\n",        instrument->panEnvelope.flags&XM_ENVELOPE_POINT_COUNT_MASK);
		XM_Printf("\t\tEnabled: %s\n",      (instrument->panEnvelope.flags&XM_ENVELOPE_ENABLED)         ? "Yes" : "No");
		XM_Printf("\t\tSustain: %s\n",      (instrument->panEnvelope.flags&XM_ENVELOPE_SUSTAIN_ENABLED) ? "Yes" : "No");
		XM_Printf("\t\tLoop Enabled: %s\n", (instrument->panEnvelope.flags&XM_ENVELOPE_LOOP_ENABLED)    ? "Yes" : "No");
		
		// Samples
		
		// Allocate array
		instrument->samples = (XMSample*)XMUtil::getMemoryManager()->alloc(sizeof(XMSample) * samples, XMMemoryManager::AllocType_SongHeader);
		if (instrument->samples == 0)
		{
			// Out of Memory
			p_song->clear(-1, i);
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		// allocate samples
		XMUtil::getMemoryManager()->zeroMem(instrument->samples, sizeof(XMSample) * samples);
		{
			XMSampleData sampleData[XMConstant_MaxSamplesPerInstrument];
			
			XM_Printf("\tSample_Names:\n");
			
			// First pass, Sample Headers
			for (int j = 0; j < samples; ++j)
			{
				XMSample* sample = &instrument->samples[j];
				
				u32 length       = m_fileIO->getU32(); // in bytes
				u32 loopStart    = m_fileIO->getU32();
				u32 loopEnd      = m_fileIO->getU32();
				sample->volume   = m_fileIO->getU8();
				sample->finetune = (s8)m_fileIO->getU8();
				u8 flags         = m_fileIO->getU8();
				sample->pan      = m_fileIO->getU8();
				sample->baseNote = (s8)m_fileIO->getU8();
				
				XM_Printf("\tSample %i:, length %i\n", j, length);
				
				if (length > 0)
				{
					// Sample data
					bool padSampleMem = (m_player->getMixer()->getRestrictions() & XMMixer::Restriction_NeedsEndPadding) != 0;
					
					switch ((flags >> 3) & 0x3)
					{
						// bit 3 of XM sample flags specify extended (proprietary) format
						case 0: sampleData[j].format = XMSampleFormat_PCM8;      break;
						case 2: sampleData[j].format = XMSampleFormat_PCM16;     break;
						case 1: sampleData[j].format = XMSampleFormat_IMA_ADPCM; break;
						case 3: sampleData[j].format = XMSampleFormat_Custom;    break;
					}
					switch (flags & 0x3)
					{
						case 3: // fallthrough
						case 0: sampleData[j].loopType = XMLoopType_Disabled; break;
						case 1: sampleData[j].loopType = XMLoopType_Forward;  break;
						case 2: sampleData[j].loopType = XMLoopType_PingPong; break;
					}
					
					sampleData[j].loopBegin = loopStart;
					sampleData[j].loopEnd   = loopEnd;
					sampleData[j].length    = length;
					
					if (sampleData[j].format == XMSampleFormat_PCM16)
					{
						// cut values in half, since length is in bytes
						sampleData[j].length    /= 2;
						sampleData[j].loopBegin /= 2;
						sampleData[j].loopEnd   /= 2;
					}
					
					sampleData[j].loopEnd += sampleData[j].loopBegin;
					
					sampleData[j].data = XMUtil::getMemoryManager()->alloc(length + (padSampleMem ? 4 : 0), XMMemoryManager::AllocType_Sample);
					
					if (sampleData[j].data == 0)
					{
						// Out of Memory
						p_song->clear(-1, i);
						m_fileIO->close();
						
						return Error_OutOfMemory;
					}
				}
				else
				{
					XM_Printf("\t\tSkipped!\n");
					
					sample->sampleId   = XMConstant_InvalidSampleID;
					sampleData[j].data = 0;
				}
				
			}
			
			// Second pass, Sample Data
			for (int j = 0; j < samples; ++j)
			{
				XMSample* sample = &instrument->samples[j];
				bool padSampleMem = (m_player->getMixer()->getRestrictions() & XMMixer::Restriction_NeedsEndPadding) != 0;
				
				if (sampleData[j].data == 0)
				{
					// no data in sample, skip it
					continue;
				}
				
				switch (sampleData[j].format)
				{
				case XMSampleFormat_PCM16:
					{
						size_t size = static_cast<size_t>((sampleData[j].length / 2) + 2);
						u8* adpcm = new u8[size];
						m_fileIO->getByteArray(adpcm, static_cast<u32>(size));
						
						s16* data = (s16*)sampleData[j].data;
						
						const u8* scratch = adpcm;
						u8 encodedSample = 0;
						bool leftOver = false;
						
						tt::audio::codec::ttadpcm::ADPCMState state;
						state.predictor = 0;
						state.stepIndex = 0;
						
						for (int k = 0; k < static_cast<int>(sampleData[j].length); ++k)
						{
							if (k == 0)
							{
								s16 readSample = code::bufferutils::get<s16>(scratch, size);
								data[k] = readSample;
								state.predictor = readSample;
								state.stepIndex = 0;
							}
							else
							{
								if (leftOver)
								{
									s16 readSample = codec::ttadpcm::decode(state, static_cast<u8>(encodedSample >> 4));
									data[k] = readSample;
									leftOver = false;
								}
								else
								{
									encodedSample = *scratch;
									++scratch;
									s16 readSample = codec::ttadpcm::decode(state, static_cast<u8>(encodedSample & 0x0F));
									data[k] = readSample;
									leftOver = true;
								}
							}
						}
						delete[] adpcm;
						
						if (padSampleMem)
						{
							// interpolation helper, these make looping smoother
							switch (sampleData[j].loopType)
							{
							case XMLoopType_Disabled:
								data[sampleData[j].length] = 0;
								break;
								
							case XMLoopType_Forward:
								data[sampleData[j].length] = data[sampleData[j].loopBegin];
								break;
								
							case XMLoopType_PingPong:
								data[sampleData[j].length] = data[sampleData[j].loopEnd];
								break;
							}
							data[sampleData[j].length + 1] = 0;
						}
					}
					break;
					
				case XMSampleFormat_PCM8:
					{
						size_t size = static_cast<size_t>((sampleData[j].length / 2) + 2);
						u8* adpcm = new u8[size];
						m_fileIO->getByteArray(adpcm, static_cast<u32>(size));
						
						s8* data = (s8*)sampleData[j].data;
						
						const u8* scratch = adpcm;
						u8 encodedSample = 0;
						bool leftOver = false;
						
						tt::audio::codec::ttadpcm::ADPCMState state;
						state.predictor = 0;
						state.stepIndex = 0;
						
						for (int k = 0; k < static_cast<int>(sampleData[j].length); ++k)
						{
							if (k == 0)
							{
								s16 readSample = code::bufferutils::get<s16>(scratch, size);
								data[k] = static_cast<s8>(readSample / 256);
								state.predictor = readSample;
								state.stepIndex = 0;
							}
							else
							{
								if (leftOver)
								{
									s16 readSample = codec::ttadpcm::decode(state, static_cast<u8>(encodedSample >> 4));
									data[k] = static_cast<s8>(readSample / 256);
									leftOver = false;
								}
								else
								{
									encodedSample = *scratch;
									++scratch;
									s16 readSample = codec::ttadpcm::decode(state, static_cast<u8>(encodedSample & 0x0F));
									data[k] = static_cast<s8>(readSample / 256);
									leftOver = true;
								}
							}
						}
						delete[] adpcm;
						
						if (padSampleMem)
						{
							// interpolation helper, these make looping smoother
							switch (sampleData[j].loopType)
							{
							case XMLoopType_Disabled:
								data[sampleData[j].length] = 0;
								break;
								
							case XMLoopType_Forward:
								data[sampleData[j].length] = data[sampleData[j].loopBegin];
								break;
								
							case XMLoopType_PingPong:
								data[sampleData[j].length] = data[sampleData[j].loopEnd];
								break;
							}
							
							data[sampleData[j].length + 1] = 0;
						}
					}
					break;
					
				default:
					{
						// just read it into memory
						m_fileIO->getByteArray((u8*)sampleData[j].data, sampleData[j].length);
					}
					break;
				}
				
				sample->sampleId = m_player->getMixer()->sampleRegister(&sampleData[j]);
				
			#ifdef _XM_DEBUG
				
				XM_Printf("\t\tLength: %i\n", sampleData[j].length);
				switch (sampleData[j].loop_type)
				{
					case XM_LOOP_DISABLED:  XM_Printf("\t\tLoop: Disabled\n"); break;
					case XM_LOOP_FORWARD:   XM_Printf("\t\tLoop: Forward\n");  break;
					case XM_LOOP_PING_PONG: XM_Printf("\t\tLoop: PingPong\n"); break;
				}
				
				XM_Printf("\t\tLoop Begin: %i\n", sampleData[j].loop_begin);
				XM_Printf("\t\tLoop End: %i\n", sampleData[j].loop_end);
				XM_Printf("\t\tVolume: %i\n", sample->volume);
				XM_Printf("\t\tPan: %i\n", sample->pan);
				XM_Printf("\t\tBase Note: %i\n", sample->base_note);
				XM_Printf("\t\tFineTune: %i\n", sample->finetune);
				
				switch (sampleData[j].format)
				{
					/* bit 3 of XM sample flags specify extended (proprietary) format */
					case XM_SAMPLE_FORMAT_PCM8:      XM_Printf("\t\tFormat: PCM8\n");      break;
					case XM_SAMPLE_FORMAT_PCM16:     XM_Printf("\t\tFormat: PCM16\n");     break;
					case XM_SAMPLE_FORMAT_IMA_ADPCM: XM_Printf("\t\tFormat: IMA_ADPCM\n"); break;
					case XM_SAMPLE_FORMAT_CUSTOM:    XM_Printf("\t\tFormat: CUSTOM\n");    break;
				}
			#endif
			}
		}
	}
	
	m_fileIO->close();
	return Error_Ok;
}


const char* XMLoader::getErrorDescription(Error p_error)
{
	switch (p_error)
	{
	case Error_Ok:               return "No error";
	case Error_Unconfigured:     return "Loader unconfigured";
	case Error_FileIOInUse:      return "File in use";
	case Error_FileCantOpen:     return "Can't open file";
	case Error_FileUnrecognized: return "File unrecognized";
	case Error_OutOfMemory:      return "Out of memory";
	case Error_FileCorrupt:      return "File is corrupt";
	default: return "Unknown error";
	}
}


// Private functions

XMLoader::Error XMLoader::openSongCustom(const char* p_fileName,
                                         XMSong*     p_song,
                                         bool        p_loadMusic,
                                         bool        p_loadInstruments)
{
	XM_Printf("\n*** LOADING XM: %s ***\n\n", p_filename);
	
	if (p_loadMusic == false && p_loadInstruments == false)
	{
		// nothing to do
		return Error_Ok;
	}
	
	if (m_fileIO                   == 0 ||
	    m_player                   == 0 ||
	    m_player->getMixer()       == 0 ||
	    XMUtil::getMemoryManager() == 0)
	{
		// Check whether we have everything needed to go
		TT_WARNING(m_fileIO != 0, "MISSING FILEIO");
		TT_WARNING(m_player != 0, "MISSING PLAYER");
		TT_WARNING(m_player != 0 && m_player->getMixer() != 0, "MISSING MIXER");
		TT_WARNING(XMUtil::getMemoryManager() != 0, "MISSING MEMORY MANAGER");
		
		return Error_Unconfigured;
	}
	
	if (m_fileIO->inUse())
	{
		return Error_FileIOInUse;
	}
		
	if (m_fileIO->open(p_fileName, false) == XMFileIO::IOError_CantOpen)
	{
		return Error_FileCantOpen;
	}
	
	p_song->setPlayer(m_player);
	
	if (p_loadMusic)
	{
		if (p_loadInstruments)
		{
			p_song->freeSong();
		}
		else
		{
			p_song->freeMusic();
		}
	}
	else if (p_loadInstruments)
	{
		
		p_song->freeInstruments();
	}
	
	// Load identifier
	{
		
		u8 idtext[18];
		m_fileIO->getByteArray(idtext, 17);
		idtext[17]=0;
		
		// TODO: validate identifier
	}
	
	// Load header
	u16 patterncount;
	{
		m_fileIO->getByteArray((u8*)p_song->name, 20);
		
		p_song->name[20] = 0;
		
		XM_Printf("Song Name: %s\n", p_song->name);
		
		u8 hex1a = m_fileIO->getU8();
		if (hex1a != 0x1A)
		{
			// XM "magic" byte
			m_fileIO->close();
			
			return Error_FileUnrecognized;
		}
		
		for (int i = 0; i < 20; ++i)
		{
			// skip trackername
			m_fileIO->getU8();
		}
		
		u16 version    = m_fileIO->getU16();
		(void)version;
		u32 headersize = m_fileIO->getU32();
		
		if (p_loadMusic)
		{
			p_song->orderCount = (u8)(m_fileIO->getU16());
			
			p_song->restartPos = (u8)(m_fileIO->getU16());
			if (p_song->restartPos >= p_song->orderCount)
			{
				p_song->restartPos = 0;
			}
			
			u8 chans = (u8)(m_fileIO->getU16());
			if (chans > XM_MaxChannels)
			{
				TT_WARN("Invalid Number of Channels: %i > %i", chans, XM_MaxChannels);
				return Error_FileCorrupt;
			}
			
			// use 5 bits, +1
			p_song->flags = (u8)((chans - 1) & 0x1F);
			patterncount  = p_song->patternCount = (u8)m_fileIO->getU16();
			
			if (p_loadInstruments)
			{
				p_song->instrumentCount = (u8)m_fileIO->getU16();
			}
			else
			{
				// ignore
				m_fileIO->getU16();
			}
			
			// flags. only linear periods
			if (m_fileIO->getU16() != 0)
			{
				p_song->flags |= XMSong::Flag_LinearPeriods;
			}
			
			p_song->speed = m_fileIO->getU16();
			p_song->tempo = m_fileIO->getU16();
			
			m_fileIO->getByteArray(p_song->orderList, 256);
			
			XM_Printf("Song Header:\n");
			XM_Printf("\tChannels: %i\n", chans);
			XM_Printf("\tOrders: %i\n", p_song->orderCount);
			XM_Printf("\tPatterns: %i\n", p_song->patternCount);
			XM_Printf("\tInstruments: %i\n", p_song->instrumentCount);
			XM_Printf("\tRestart At: %i\n", p_song->restartPos);
			XM_Printf("\tTempo: %i\n", p_song->tempo);
			XM_Printf("\tSpeed: %i\n", p_song->speed);
			XM_Printf("\tOrders: ");
			
			for (int i = 0; i < p_song->orderCount; ++i)
			{
				if (i > 0)
				{
					XM_Printf(", ");
				}
				XM_Printf("%i", p_song->orderList[i]);
			}
			
			XM_Printf("\n");
		}
		else
		{
			// skip order count
			m_fileIO->getU16();
			
			// skip restart pos
			m_fileIO->getU16();
			
			// skip flags
			m_fileIO->getU16();
			
			// skip pattern count
			patterncount = m_fileIO->getU16();
			p_song->instrumentCount = (u8)m_fileIO->getU16();
			
			// skip to end of header
			XM_Printf("Skipping Header.. \n");
			XM_Printf("\tInstruments: %i \n", p_song->instrumentCount);
		}
		
		while (m_fileIO->getPos() < (headersize + 60))
		{
			// finish reading header
			m_fileIO->getU8();
		}
	}
	
	// Load patterns
	if (p_loadMusic && p_song->patternCount != 0)
	{
		p_song->patternData = (u8**)XMUtil::getMemoryManager()->alloc(sizeof(u8*)*p_song->patternCount, XMMemoryManager::AllocType_Pattern);
		
		if (p_song->patternData == 0)
		{
			// Handle OUT OF MEMORY
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		XMUtil::getMemoryManager()->zeroMem(p_song->patternData, sizeof(u8*) * p_song->patternCount);
	}
	
	XM_Printf("\n\n");
	
	for (int i = 0; i < patterncount; ++i)
	{
		u32 offset         = m_fileIO->getPos(); // current file position
		u32 headerLength   = m_fileIO->getU32(); // pattern header length
		u8  packing        = m_fileIO->getU8();  // if packing = 1, this pattern is pre-packed
		u16 rows           = m_fileIO->getU16(); // rows
		u16 packedDataSize = m_fileIO->getU16(); // pattern header length
		
		XM_Printf("Pattern: %i\n", i);
		XM_Printf("\tHeader Len: %i\n", headerLength);
		XM_Printf("\tRows: %i\n", rows);
		XM_Printf("\tPacked Size: %i\n", packedDataSize);
		
		while (m_fileIO->getPos() < (offset + headerLength))
		{
			// finish reading header
			m_fileIO->getU8();
		}
		
		if (p_loadMusic)
		{
			if (packedDataSize == 0)
			{
				p_song->patternData[i] = 0;
			}
			else if (packing == 1)
			{
				// pre packed pattern
				u8* pdata = (u8*)XMUtil::getMemoryManager()->alloc(packedDataSize, XMMemoryManager::AllocType_Pattern);
				if (pdata == 0)
				{
					// Handle OUT OF MEMORY
					p_song->clear(i, -1);
					m_fileIO->close();
					
					return Error_OutOfMemory;
				}
				
				m_fileIO->getByteArray(pdata, packedDataSize);
				
				p_song->patternData[i] = pdata;
			}
			else
			{
				// TODO: read all sample data from file and recompress using XMCompressor class
				
				// pack on the fly while reading
				u32 packBeginPos = m_fileIO->getPos();
				
				// just calculate size
				u32 repackedSize = recompressPattern(rows, (u8)((p_song->flags & 0x1f) + 1), 0);
				
				XM_Printf("\tRePacked Size: %i\n", repackedSize);
				
				m_fileIO->seekPos(packBeginPos);
				
				u8* pdata = (u8*)XMUtil::getMemoryManager()->alloc(1 + repackedSize, XMMemoryManager::AllocType_Pattern);
				
				if (pdata == 0)
				{
					// Handle OUT OF MEMORY
					p_song->clear(i, -1);
					m_fileIO->close();
					
					return Error_OutOfMemory;
				}
				
				// first byte is rows
				pdata[0] = (u8)(rows - 1);
				
				// on the fly recompress
				recompressPattern(rows, (u8)((p_song->flags & 0x1f) + 1), &pdata[1]);
				
				p_song->patternData[i] = pdata;
			}
		}
		
		while (m_fileIO->getPos() < (offset + headerLength + packedDataSize))
		{
			// finish reading header
			m_fileIO->getU8();
		}
	}
	
	// Load instruments
	
	if (p_loadInstruments && p_song->instrumentCount != 0)
	{
		p_song->instrumentData = (XMInstrument**)XMUtil::getMemoryManager()->alloc(sizeof(XMInstrument*) * p_song->instrumentCount, XMMemoryManager::AllocType_Instrument);
		
		if (p_song->instrumentData == 0)
		{
			// Handle OUT OF MEMORY
			p_song->clear(p_loadMusic ? p_song->patternCount : -1, 0);
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		XMUtil::getMemoryManager()->zeroMem(p_song->instrumentData, sizeof(XMInstrument*) * p_song->instrumentCount);
	}
	else
	{
		// Don't load instruments
		m_fileIO->close();
		
		return Error_Ok;
	}
	
	for (int i = 0; i < p_song->instrumentCount; ++i)
	{
		u32 offset       = m_fileIO->getPos();
		u32 headerLength = m_fileIO->getU32();
		
		for (int j = 0; j < 22; ++j)
		{
			// skip name
			m_fileIO->getU8();
		}
		
		// ignore type
		m_fileIO->getU8();
		
		u16 samples = m_fileIO->getU16();
		
		XM_Printf("Instrument: %i\n", i);
		XM_Printf("\tHeader Len: %i\n", headerLength);
		
		if (samples == 0)
		{
			// empty instrument
			p_song->instrumentData[i] = 0;
			
			XM_Printf("\tSkipped!\n");
			
			while ((m_fileIO->getPos() - offset) < headerLength)
			{
				// go to end of header
				m_fileIO->getU8();
			}
			continue;
			// TODO: goto header len
		}
		else if (samples > XMConstant_MaxSamplesPerInstrument)
		{
			TT_WARN("\tHas invalid sample count: %i\n", samples);
			
			p_song->clear(p_loadMusic ? p_song->patternCount : -1, i);
			m_fileIO->close();
			
			return Error_FileCorrupt;
		}
		else
		{
			p_song->instrumentData[i] = (XMInstrument*)XMUtil::getMemoryManager()->alloc(sizeof(XMInstrument), XMMemoryManager::AllocType_Instrument);
			
			if (p_song->instrumentData[i] == 0)
			{
				// Out of Memory
				p_song->clear(p_loadMusic ? p_song->patternCount : -1, i);
				m_fileIO->close();
				
				return Error_OutOfMemory;
			}
			
			XMUtil::getMemoryManager()->zeroMem(p_song->instrumentData[i], sizeof(XMInstrument));
		}
		
		XMInstrument* instrument = p_song->instrumentData[i];
		instrument->sampleCount  = (u8)samples;
		
		// reset the samples
		instrument->samples = 0;
		
		// "sample header size" is redundant, so I ignore it
		u32 sampleHeaderSize = m_fileIO->getU32();
		(void)sampleHeaderSize;
		XM_Printf("\tSample Header Size: %i\n", sampleHeaderSize);
		
		for (int j = 0; j < 48; ++j)
		{
			// convert to nibbles
			u8 nibble = (u8)(m_fileIO->getU8() & 0xF);
			nibble   |= m_fileIO->getU8() << 4;
			
			instrument->noteSample[j] = nibble;
		}
		
		for (int j = 0; j < 12; ++j)
		{
			u16 ofs = m_fileIO->getU16();
			u16 val = m_fileIO->getU16();
			
			// encode into 16 bits
			instrument->volumeEnvelope.points[j] = (u16)((val << 9) | ofs);
		}
		for (int j = 0; j < 12; ++j)
		{
			u16 ofs = m_fileIO->getU16();
			u16 val = m_fileIO->getU16();
			
			// encode into 16 bits
			instrument->panEnvelope.points[j] = (u16)((val << 9) | ofs);
		}
		
		instrument->volumeEnvelope.flags = m_fileIO->getU8();
		instrument->panEnvelope.flags    = m_fileIO->getU8();
		
		instrument->volumeEnvelope.sustainIndex   = m_fileIO->getU8();
		instrument->volumeEnvelope.loopBeginIndex = m_fileIO->getU8();
		instrument->volumeEnvelope.loopEndIndex   = m_fileIO->getU8();
		
		instrument->panEnvelope.sustainIndex   = m_fileIO->getU8();
		instrument->panEnvelope.loopBeginIndex = m_fileIO->getU8();
		instrument->panEnvelope.loopEndIndex   = m_fileIO->getU8();
		
		{
			// Volume Envelope Flags
			u8 envelopeFlags = m_fileIO->getU8();
			if ((envelopeFlags & 1) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_Enabled;
			}
			if ((envelopeFlags & 2) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_SustainEnabled;
			}
			if ((envelopeFlags & 4) != 0)
			{
				instrument->volumeEnvelope.flags |= XMEnvelope::Flag_LoopEnabled;
			}
		}
		{
			// Panning Envelope Flags
			u8 envelopeFlags = m_fileIO->getU8();
			
			if ((envelopeFlags & 1) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_Enabled;
			}
			if ((envelopeFlags & 2) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_SustainEnabled;
			}
			if ((envelopeFlags & 4) != 0)
			{
				instrument->panEnvelope.flags |= XMEnvelope::Flag_LoopEnabled;
			}
		}
		
		instrument->vibratoType  = (XMVibratoType)m_fileIO->getU8();
		instrument->vibratoSweep = m_fileIO->getU8();
		instrument->vibratoDepth = m_fileIO->getU8();
		instrument->vibratoRate  = m_fileIO->getU8();
		instrument->fadeout      = m_fileIO->getU16();
		
		// reserved
		m_fileIO->getU16();
		
		XM_Printf("\tVolume Envelope:\n");
		XM_Printf("\t\tPoints: %i\n",        instrument->volumeEnvelope.flags&XM_ENVELOPE_POINT_COUNT_MASK);
		XM_Printf("\t\tEnabled: %s\n",      (instrument->volumeEnvelope.flags&XM_ENVELOPE_ENABLED) ?         "Yes" : "No");
		XM_Printf("\t\tSustain: %s\n",      (instrument->volumeEnvelope.flags&XM_ENVELOPE_SUSTAIN_ENABLED) ? "Yes" : "No");
		XM_Printf("\t\tLoop Enabled: %s\n", (instrument->volumeEnvelope.flags&XM_ENVELOPE_LOOP_ENABLED) ?    "Yes" : "No");
		
		XM_Printf("\tPan Envelope:\n");
		XM_Printf("\t\tPoints: %i\n",        instrument->panEnvelope.flags&XM_ENVELOPE_POINT_COUNT_MASK);
		XM_Printf("\t\tEnabled: %s\n",      (instrument->panEnvelope.flags&XM_ENVELOPE_ENABLED)         ? "Yes" : "No");
		XM_Printf("\t\tSustain: %s\n",      (instrument->panEnvelope.flags&XM_ENVELOPE_SUSTAIN_ENABLED) ? "Yes" : "No");
		XM_Printf("\t\tLoop Enabled: %s\n", (instrument->panEnvelope.flags&XM_ENVELOPE_LOOP_ENABLED)    ? "Yes" : "No");
		
		while ((m_fileIO->getPos() - offset) < headerLength)
		{
			// Skip rest of header
			m_fileIO->getU8();
		}
		
		// Samples
		
		// Allocate array
		instrument->samples = (XMSample*)XMUtil::getMemoryManager()->alloc(sizeof(XMSample) * samples, XMMemoryManager::AllocType_SongHeader);
		
		if (instrument->samples == 0)
		{
			// Out of Memory
			p_song->clear(p_loadMusic ? p_song->patternCount : -1, i);
			m_fileIO->close();
			
			return Error_OutOfMemory;
		}
		
		// allocate samples
		XMUtil::getMemoryManager()->zeroMem(instrument->samples, sizeof(XMSample) * samples);
		{
			XMSampleData sampleData[XMConstant_MaxSamplesPerInstrument];
			
			XM_Printf("\tSample_Names:\n");
			
			// First pass, Sample Headers
			for (int j = 0; j < samples; ++j)
			{
				XMSample* sample = &instrument->samples[j];
				
				u32 length       = m_fileIO->getU32(); // in bytes
				u32 loopStart    = m_fileIO->getU32();
				u32 loopEnd      = m_fileIO->getU32();
				sample->volume   = m_fileIO->getU8();
				sample->finetune = (s8)m_fileIO->getU8();
				u8 flags         = m_fileIO->getU8();
				sample->pan      = m_fileIO->getU8();
				sample->baseNote = (s8)m_fileIO->getU8();
				
				// reserved
				m_fileIO->getU8();
				
			#ifdef XM_DEBUG
				XM_Printf("\t\t%i- ", j);
				for (int k = 0; k < 22; ++k)
				{
					XM_Printf("%c", m_fileIO->get_u8());
				}
				XM_Printf("\n");
			#else
				for (int k = 0; k < 22; ++k)
				{
					// skip sample name
					m_fileIO->getU8();
				}
			#endif
				
				XM_Printf("\tSample %i:, length %i\n", j, length);
				
				if (length > 0)
				{
					// Sample data
					bool padSampleMem = (m_player->getMixer()->getRestrictions() & XMMixer::Restriction_NeedsEndPadding) != 0;
					
					switch ((flags >> 3) & 0x3)
					{
						// bit 3 of XM sample flags specify extended (proprietary) format
						case 0: sampleData[j].format = XMSampleFormat_PCM8;      break;
						case 2: sampleData[j].format = XMSampleFormat_PCM16;     break;
						case 1: sampleData[j].format = XMSampleFormat_IMA_ADPCM; break;
						case 3: sampleData[j].format = XMSampleFormat_Custom;    break;
					}
					switch (flags & 0x3)
					{
						case 3: // fallthrough
						case 0: sampleData[j].loopType = XMLoopType_Disabled; break;
						case 1: sampleData[j].loopType = XMLoopType_Forward;  break;
						case 2: sampleData[j].loopType = XMLoopType_PingPong; break;
					}
					
					sampleData[j].loopBegin = loopStart;
					sampleData[j].loopEnd   = loopEnd;
					sampleData[j].length    = length;
					
					if (sampleData[j].format == XMSampleFormat_PCM16)
					{
						// cut values in half, since length is in bytes
						sampleData[j].length    /= 2;
						sampleData[j].loopBegin /= 2;
						sampleData[j].loopEnd   /= 2;
					}
					
					sampleData[j].loopEnd += sampleData[j].loopBegin;
					
					sampleData[j].data = XMUtil::getMemoryManager()->alloc(length + (padSampleMem ? 4 : 0), XMMemoryManager::AllocType_Sample);
					
					if (sampleData[j].data == 0)
					{
						// Out of Memory
						p_song->clear(p_loadMusic ? p_song->patternCount : -1, i);
						m_fileIO->close();
						
						return Error_OutOfMemory;
					}
				}
				else
				{
					XM_Printf("\t\tSkipped!\n");
					
					sample->sampleId   = XMConstant_InvalidSampleID;
					sampleData[j].data = 0;
				}
				
			}
			
			// Second pass, Sample Data
			for (int j = 0; j < samples; ++j)
			{
				XMSample* sample = &instrument->samples[j];
				bool padSampleMem = (m_player->getMixer()->getRestrictions() & XMMixer::Restriction_NeedsEndPadding) != 0;
				
				if (sampleData[j].data == 0)
				{
					// no data in sample, skip it
					continue;
				}
				
				switch (sampleData[j].format)
				{
				case XMSampleFormat_PCM16:
					{
						s16* data = (s16*)sampleData[j].data;
						s16 old = 0;
						for (int k = 0; k < (int)sampleData[j].length; ++k)
						{
							s16 d   = (s16)m_fileIO->getU16();
							data[k] = (s16)(d + old);
							old     = data[k];
						}
						if (padSampleMem)
						{
							// interpolation helper, these make looping smoother
							switch (sampleData[j].loopType)
							{
							case XMLoopType_Disabled:
								data[sampleData[j].length] = 0;
								break;
								
							case XMLoopType_Forward:
								data[sampleData[j].length] = data[sampleData[j].loopBegin];
								break;
								
							case XMLoopType_PingPong:
								data[sampleData[j].length] = data[sampleData[j].loopEnd];
								break;
							}
							data[sampleData[j].length + 1] = 0;
						}
					}
					break;
					
				case XMSampleFormat_PCM8:
					{
						s8* data = (s8*)sampleData[j].data;
						s8 old = 0;
						for (int k = 0; k < (int)sampleData[j].length; ++k)
						{
							s8 d    = (s8)m_fileIO->getU8();
							data[k] = (s8)(d + old);
							old     = data[k];
						}
						
						if (padSampleMem)
						{
							// interpolation helper, these make looping smoother
							switch (sampleData[j].loopType)
							{
							case XMLoopType_Disabled:
								data[sampleData[j].length] = 0;
								break;
								
							case XMLoopType_Forward:
								data[sampleData[j].length] = data[sampleData[j].loopBegin];
								break;
								
							case XMLoopType_PingPong:
								data[sampleData[j].length] = data[sampleData[j].loopEnd];
								break;
							}
							
							data[sampleData[j].length + 1] = 0;
						}
					}
					break;
					
				default:
					{
						// just read it into memory
						m_fileIO->getByteArray((u8*)sampleData[j].data, sampleData[j].length);
					}
					break;
				}
				
				sample->sampleId = m_player->getMixer()->sampleRegister(&sampleData[j]);
				
			#ifdef _XM_DEBUG
				
				XM_Printf("\t\tLength: %i\n", sampleData[j].length);
				switch (sampleData[j].loop_type)
				{
					case XM_LOOP_DISABLED:  XM_Printf("\t\tLoop: Disabled\n"); break;
					case XM_LOOP_FORWARD:   XM_Printf("\t\tLoop: Forward\n");  break;
					case XM_LOOP_PING_PONG: XM_Printf("\t\tLoop: PingPong\n"); break;
				}
				
				XM_Printf("\t\tLoop Begin: %i\n", sampleData[j].loop_begin);
				XM_Printf("\t\tLoop End: %i\n", sampleData[j].loop_end);
				XM_Printf("\t\tVolume: %i\n", sample->volume);
				XM_Printf("\t\tPan: %i\n", sample->pan);
				XM_Printf("\t\tBase Note: %i\n", sample->base_note);
				XM_Printf("\t\tFineTune: %i\n", sample->finetune);
				
				switch (sampleData[j].format)
				{
					/* bit 3 of XM sample flags specify extended (proprietary) format */
					case XM_SAMPLE_FORMAT_PCM8:      XM_Printf("\t\tFormat: PCM8\n");      break;
					case XM_SAMPLE_FORMAT_PCM16:     XM_Printf("\t\tFormat: PCM16\n");     break;
					case XM_SAMPLE_FORMAT_IMA_ADPCM: XM_Printf("\t\tFormat: IMA_ADPCM\n"); break;
					case XM_SAMPLE_FORMAT_CUSTOM:    XM_Printf("\t\tFormat: CUSTOM\n");    break;
				}
			#endif
			}
		}
	}
	
	m_fileIO->close();
	return Error_Ok;
}


/**
 * Recompress Pattern from XM-Compression to Custom (smaller/faster to read) compression
 * Read pattern.txt included with this file for the format.
 * @param p_rows Rows for source pattern
 * @param channels Channels for source pattern
 * @param p_dst_data recompress target pointer, if null, will just compute size
 * @return size of recompressed buffer
 */

u32 XMLoader::recompressPattern(u16 p_rows, u8 p_channels, void* p_dst_data)
{
	
#define _XM_COMP_ADD_BYTE(m_b) \
{ \
	if ( dst_data != 0 )\
	{\
		dst_data[data_size] = m_b;\
	}\
	++data_size;\
}\

	XMFileIO* f = m_fileIO;
	
	u8* dst_data  = (u8*)p_dst_data;
	u32 data_size = 0;
	
	u8 caches[XM_MaxChannels][5];
	s8 last_channel = -1;
	u16 last_row    = 0;
	
	for ( int i = 0; i < XM_MaxChannels; ++i )
	{
		caches[i][0] = XM_FieldEmpty;
		caches[i][1] = XM_FieldEmpty;
		caches[i][2] = XM_FieldEmpty;
		caches[i][3] = XM_FieldEmpty;
		caches[i][4] = 0; /* only values other than 0 are read for this as cache */
	}
	
	for( int j = 0; j < p_rows; ++j )
	{
		for( int k = 0; k < p_channels; ++k )
		{
			u8 note       = XM_FieldEmpty;
			u8 instrument = 0; /* Empty XM Instrument */
			u8 volume     = 0; /* Empty XM Volume */
			u8 command    = XM_FieldEmpty;
			u8 parameter  = 0;
			u8 arb        = 0; /* advance one row bit */
			
			u8 aux_byte = 0;
			
			u8 cache_bits     = 0;
			u8 new_field_bits = 0;
			
			aux_byte          = f->getU8();
			
			if ( (aux_byte & 0x80) == 0 )
			{
				note     = aux_byte;
				aux_byte = 0xFE; /* if bit 7 not set, read all of them except the note */
			}
			
			if ( (aux_byte & 1) != 0 )
			{
				note = f->getU8();
			}
			if ( (aux_byte & 2) != 0 )
			{
				instrument = f->getU8();
			}
			if ( (aux_byte & 4) != 0 )
			{
				volume = f->getU8();
			}
			if ( (aux_byte & 8) != 0 )
			{
				command = f->getU8();
			}
			if ( (aux_byte & 16) != 0 )
			{
				parameter = f->getU8();
			}
			
			if ( note > 97 )
			{
				note = XM_FieldEmpty;
			}
			
			if ( instrument == 0 )
			{
				instrument = XM_FieldEmpty;
			}
			else
			{
				--instrument;
			}
			
			if ( volume < 0x10 )
			{
				volume = XM_FieldEmpty;
			}
			else
			{
				volume -= 0x10;
			}
			
			if ( command == 0 && parameter == 0 )
			{
				/* this equals to nothing */
				command = XM_FieldEmpty;
			}
			
			/** COMPRESS!!! **/
			
			/* Check differences with cache and place them into bits */
			cache_bits |= (note       != XM_FieldEmpty && note       == caches[k][0]) ? XM_CompNoteBit       : 0;
			cache_bits |= (instrument != XM_FieldEmpty && instrument == caches[k][1]) ? XM_CompInstrumentBit : 0;
			cache_bits |= (volume     != XM_FieldEmpty && volume     == caches[k][2]) ? XM_CompVolumeBit     : 0;
			cache_bits |= (command    != XM_FieldEmpty && command    == caches[k][3]) ? XM_CompCommandBit    : 0;
			cache_bits |= (parameter  != 0             && parameter  == caches[k][4]) ? XM_CompParameterBit  : 0;
			
			/* Check new field values and place them into bits and cache*/
			
			if ( note != XM_FieldEmpty && (cache_bits & XM_CompNoteBit) == 0 )
			{
				new_field_bits |= XM_CompNoteBit;
				caches[k][0] = note;
			}
			
			if ( instrument != XM_FieldEmpty && (cache_bits & XM_CompInstrumentBit) == 0 )
			{
				new_field_bits |= XM_CompInstrumentBit;
				caches[k][1] = instrument;
			}
			
			if ( volume != XM_FieldEmpty && (cache_bits & XM_CompVolumeBit) == 0 )
			{
				new_field_bits |= XM_CompVolumeBit;
				caches[k][2] = volume;
				
			}
			
			if ( command != XM_FieldEmpty && (cache_bits & XM_CompCommandBit) == 0)
			{
				new_field_bits |= XM_CompCommandBit;
				caches[k][3] = command;
			}
			
			if ( parameter != 0 && (cache_bits & XM_CompParameterBit) == 0 )
			{
				new_field_bits |= XM_CompParameterBit;
				caches[k][4] = parameter;
			}
			
			if ( new_field_bits == 0 && cache_bits == 0 )
			{
				continue; /* nothing to store, empty field */
			}
			
			/* Seek to Row */
			
			if ( j > 0 && last_row == (j - 1) && last_channel != k )
			{
				arb = 0x80;
				last_row = (u16)j;
			}
			else
			{
				while ( last_row < j )
				{
					u16 diff = (u16)(j - last_row);
					
					if ( diff > 0x20 )
					{
						/* The maximum value of advance_rows command is 32 (0xFF)
						   so, if the rows that are needed to advance are greater than that,
						   advance 32, then issue more advance_rows commands */
						_XM_COMP_ADD_BYTE( 0xFF ); /* Advance 32 rows */
						last_row += 0x20;
					}
					else
					{
						_XM_COMP_ADD_BYTE( (u8)(0xE0 + (diff - 1)) ); /* Advance needed rows */
						last_row += diff;
					}
					
					last_channel = 0; /* advancing rows sets the last channel to zero */
				}
			}
			
			/* Seek to Channel */
			
			if ( last_channel != k )
			{
				_XM_COMP_ADD_BYTE( (u8)(arb | k) );
			}
			
			if ( cache_bits != 0 )
			{
				_XM_COMP_ADD_BYTE( (u8)(cache_bits | (1 << 5)) );
			}
			
			if ( new_field_bits != 0 )
			{
				_XM_COMP_ADD_BYTE( (u8)(new_field_bits | (2 << 5)) );
				for ( int i = 0; i < 5; ++i )
				{
					if ( new_field_bits & (1 << i) )
					{
						_XM_COMP_ADD_BYTE( caches[k][i] );
					}
				}
			}
		}
	}
	
	_XM_COMP_ADD_BYTE( 0x60 ); /* end of pattern */
	
#undef _XM_COMP_ADD_BYTE
	
	return data_size;
}

// namespace end
}
}
}
