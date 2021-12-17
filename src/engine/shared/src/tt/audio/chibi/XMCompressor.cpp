#include <tt/audio/chibi/XMCompressor.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace chibi {

XMCompressor::XMCompressor()
{
	
}


XMCompressor::~XMCompressor()
{
	
}


u32 XMCompressor::compressPattern(int p_rows, int p_channels, XMNote* p_src, u8* p_dest)
{
	u8 caches[XM_MaxChannels][5];
	for ( int i = 0; i < XM_MaxChannels; ++i )
	{
		caches[i][0] = XM_FieldEmpty;
		caches[i][1] = XM_FieldEmpty;
		caches[i][2] = XM_FieldEmpty;
		caches[i][3] = XM_FieldEmpty;
		caches[i][4] = 0; // only values other than 0 are read for this as cache
	}
	
	u32 dataSize    = 0;
	s8  lastChannel = -1;
	int lastRow     = 0;
	
	for (int j = 0; j < p_rows; ++j)
	{
		for (int k = 0; k < p_channels; ++k)
		{
			XMNote xmnote = p_src[j * p_channels + k];
			
			// sanatize input
			if (xmnote.note > XM_NoteOff) // last valid note
			{
				xmnote.note = XM_FieldEmpty;
			}
			
			if (xmnote.instrument == 0)
			{
				xmnote.instrument = XM_FieldEmpty;
			}
			else
			{
				--xmnote.instrument;
			}
			
			if (xmnote.volume < 0x10) // invalid volume
			{
				xmnote.volume = XM_FieldEmpty;
			}
			else
			{
				xmnote.volume -= 0x10;
			}
			
			if (xmnote.command == 0 && xmnote.parameter == 0)
			{
				// this equals to nothing
				xmnote.command = XM_FieldEmpty;
			}
			
			// Start compression
			
			// Check differences with cache and place them into bits
			u8 cacheBits = 0;
			cacheBits |= (xmnote.note       != XM_FieldEmpty && xmnote.note       == caches[k][0]) ? XM_CompNoteBit       : 0;
			cacheBits |= (xmnote.instrument != XM_FieldEmpty && xmnote.instrument == caches[k][1]) ? XM_CompInstrumentBit : 0;
			cacheBits |= (xmnote.volume     != XM_FieldEmpty && xmnote.volume     == caches[k][2]) ? XM_CompVolumeBit     : 0;
			cacheBits |= (xmnote.command    != XM_FieldEmpty && xmnote.command    == caches[k][3]) ? XM_CompCommandBit    : 0;
			cacheBits |= (xmnote.parameter  != 0             && xmnote.parameter  == caches[k][4]) ? XM_CompParameterBit  : 0;
			
			// Check new field values and place them into bits and cache
			u8 newFieldBits = 0;
			
			if (xmnote.note != XM_FieldEmpty && (cacheBits & XM_CompNoteBit) == 0)
			{
				newFieldBits |= XM_CompNoteBit;
				caches[k][0] = xmnote.note;
			}
			
			if (xmnote.instrument != XM_FieldEmpty && (cacheBits & XM_CompInstrumentBit) == 0)
			{
				newFieldBits |= XM_CompInstrumentBit;
				caches[k][1] = xmnote.instrument;
			}
			
			if (xmnote.volume != XM_FieldEmpty && (cacheBits & XM_CompVolumeBit) == 0)
			{
				newFieldBits |= XM_CompVolumeBit;
				caches[k][2] = xmnote.volume;
			}
			
			if (xmnote.command != XM_FieldEmpty && (cacheBits & XM_CompCommandBit) == 0)
			{
				newFieldBits |= XM_CompCommandBit;
				caches[k][3] = xmnote.command;
			}
			
			if (xmnote.parameter != 0 && (cacheBits & XM_CompParameterBit) == 0)
			{
				newFieldBits |= XM_CompParameterBit;
				caches[k][4] = xmnote.parameter;
			}
			
			if (newFieldBits == 0 && cacheBits == 0)
			{
				// nothing to store, empty field
				continue;
			}
			
			// Seek to Row
			
			u8 arb = 0; // advance row bit (arb)
			if (j > 0 && lastRow == (j - 1) && lastChannel != k)
			{
				arb     = XM_CompReadChannelAdvRow << 5;
				lastRow = j;
			}
			else
			{
				while (lastRow < j)
				{
					int diff = j - lastRow;
					
					if (diff > 0x20)
					{
						// The maximum value of advance_rows command is 32 (0xFF)
						// so, if the rows that are needed to advance are greater than that,
						// advance 32, then issue more advance_rows commands
						
						if (p_dest != 0)
						{
							p_dest[dataSize] = 0xFF;
						}
						++dataSize;
						
						lastRow += 0x20;
					}
					else
					{
						// Advance needed rows
						if (p_dest != 0)
						{
							p_dest[dataSize] = (u8)((XM_CompAdvanceRows << 5) + (diff - 1));
						}
						++dataSize;
						
						lastRow += diff;
					}
					
					// advancing rows sets the last channel to zero
					lastChannel = 0;
				}
			}
			
			// Seek to Channel
			if (lastChannel != k)
			{
				if (p_dest != 0)
				{
					p_dest[dataSize] = (u8)(arb | k);
				}
				++dataSize;
			}
			
			// write which values should be fetched from cache
			if (cacheBits != 0)
			{
				if (p_dest != 0)
				{
					p_dest[dataSize] = (u8)(cacheBits | (XM_CompUseCaches << 5));
				}
				++dataSize;
			}
			
			// write which cache values should be updated
			if (newFieldBits != 0)
			{
				if (p_dest != 0)
				{
					p_dest[dataSize] = (u8)(newFieldBits | (XM_CompReadFields << 5));
				}
				++dataSize;
				
				// write the updated cache values
				for (int i = 0; i < 5; ++i)
				{
					if (newFieldBits & (1 << i))
					{
						if (p_dest != 0)
						{
							p_dest[dataSize] = caches[k][i];
						}
						++dataSize;
					}
				}
			}
		}
	}
	
	// End of pattern
	if (p_dest != 0)
	{
		p_dest[dataSize] = XM_CompEndOfPattern << 5;
	}
	++dataSize;
	
	return dataSize;
}


} // namespace emd
}
}
