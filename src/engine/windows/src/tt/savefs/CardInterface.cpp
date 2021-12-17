#include <string>
#include <cstring>

#include <tt/savefs/CardInterface.h>
#include <tt/savefs/CardErrorHandler.h>

#include <tt/platform/tt_printf.h>
#include <tt/fs/File.h>
#include <tt/code/helpers.h>



//#define FS_DEBUG
#if !defined(TT_BUILD_FINAL) && defined(FS_DEBUG)
	#define FS_Printf TT_Printf
#else
	#define FS_Printf(...)
#endif

//#define FS_TRACE
#if !defined(TT_BUILD_FINAL) && defined(FS_TRACE)
	#define FS_Trace TT_Printf
#else
	#define FS_Trace(...)
#endif


namespace tt {
namespace savefs {

static fs::FilePtr       s_file;
static u32               s_size;
static CardErrorHandler* s_handler;
static u8*               s_mem = 0;


bool CardInterface::initialize(CardType          p_type,
                               CardErrorHandler* p_handler,
                               fs::identifier    p_baseFSType)
{
	FS_Trace("CardInterface::initialize: Card: %d\n", p_type);
	
	s_handler = p_handler;
	
	std::string filename;
	switch (p_type)
	{
	case CardType_EEPROM_4Kbit:   filename = "eeprom4.sav";   break;
	case CardType_EEPROM_64Kbit:  filename = "eeprom64.sav";  break;
	case CardType_EEPROM_521Kbit: filename = "eeprom512.sav"; break;
	case CardType_FLASH_2Mbit:    filename = "flash2.sav";    break;
	case CardType_FLASH_4Mbit:    filename = "flash4.sav";    break;
	case CardType_FLASH_8Mbit:    filename = "flash8.sav";    break;
	case CardType_FLASH_16Mbit:   filename = "flash16.sav";   break;
	case CardType_FRAM_256Kbit:   filename = "fram256.sav";   break;
	default: TT_PANIC("Unsupported type %d.", p_type); return false;
	}
	
	FS_Printf("CardInterface::initialize: using '%s'.\n", filename.c_str());
	
	if (fs::fileExists(filename.c_str(), p_baseFSType))
	{
		// open existing without destroying content
		s_file = fs::open(filename, fs::OpenMode(fs::OpenMode_Read | fs::OpenMode_Write), p_baseFSType);
		if (s_file == 0)
		{
			if (s_handler != 0)
			{
				s_handler->handleError(CardError_Read);
			}
			else
			{
				TT_PANIC("Unable to open '%s'.", filename.c_str());
			}
			return false;
		}
	}
	else
	{
		// create new
		FS_Printf("CardInterface::initialize: Creating new file.\n");
		s_file = fs::open(filename, fs::OpenMode_Write, p_baseFSType);
		if (s_file == 0)
		{
			if (s_handler != 0)
			{
				s_handler->handleError(CardError_Read);
			}
			else
			{
				TT_PANIC("Unable to open '%s'.", filename.c_str());
			}
			return false;
		}
	}
	
	s_size = getCardSize(p_type);
	fs::size_type size = static_cast<fs::size_type>(s_size);
	fs::size_type fileSize = static_cast<size_t>(s_file->getLength());
	if (size > fileSize)
	{
		// append
		fs::size_type todo = size - fileSize;
		u8* buffer = new u8[todo];
		if (buffer == 0)
		{
			TT_PANIC("Unable to allocate %d bytes.", todo);
			
			// write the oldfashioned way
			while (size > s_file->getLength())
			{
				u8 pad = 0xFF;
				fs::writeInteger(s_file, pad);
			}
		}
		else
		{
			std::memset(buffer, 0xFF, todo);
			s_file->write(buffer, todo);
			delete[] buffer;
		}
	}
	
	if (size < s_file->getLength())
	{
		FS_Printf("CardInterface::initialize; file too big, truncating.\n");
		s_file->seek(0, fs::SeekPos_Set);
		u8* buffer = new u8[s_size];
		if (s_file->read(buffer, size) != size)
		{
			TT_PANIC("Unable to read '%s' completely.", filename.c_str());
			s_file.reset();
			delete[] buffer;
			return false;
		}
		
		s_file = fs::open(filename, fs::OpenMode(fs::OpenMode_Read | fs::OpenMode_Write | fs::OpenMode_Truncate), p_baseFSType);
		if (s_file == 0)
		{
			TT_PANIC("Unable to reopen '%s'.", filename.c_str());
			delete[] buffer;
			return false;
		}
		if (s_file->write(buffer, size) != size)
		{
			TT_PANIC("Unable to write '%s' completely.", filename.c_str());
			s_file.reset();
			delete[] buffer;
			return false;
		}
		delete[] buffer;
		FS_Printf("CardInterface::initialize: truncating successful.\n");
	}
	
	s_file->seek(0, fs::SeekPos_Set);
	
	s_mem = new u8[s_size];
	if (s_mem == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", s_size);
		end();
		return false;
	}
	
	FS_Printf("CardInterface::initialize: successful.\n");
	return true;
}


bool CardInterface::end()
{
	FS_Trace("CardInterface::end\n");
	if (s_size == 0)
	{
		TT_PANIC("Card already ended or never initialized.");
		return false;
	}
	
	tt::code::helpers::safeDeleteArray(s_mem);
	s_size = 0;
	
	s_file.reset();
	
	FS_Printf("CardInterface::end: successful.\n");
	return true;
}


bool CardInterface::read(u32 p_src, void* p_dest, u32 p_len, bool p_dummy)
{
	FS_Trace("CardInterface::read: %s from %d to %p, %d bytes\n",
	         p_dummy ? "dummy " : "", p_src, p_dest, p_len);
	
	if (p_src >= s_size)
	{
		TT_PANIC("Source %d out of bounds.", p_src);
		return false;
	}
	
	if (p_src + p_len > s_size)
	{
		TT_PANIC("Source + length (%d + %d = %d) out of bounds.",
		         p_src, p_len, p_src + p_len);
		return false;
	}
	
	if (p_dest == 0)
	{
		TT_PANIC("Destination must not be null.");
		return false;
	}
	
	if (s_file->seek(fs::off_type(p_src), fs::SeekPos_Set) == false)
	{
		if (p_dummy == false)
		{
			if (s_handler != 0)
			{
				s_handler->handleError(CardError_Read);
			}
			else
			{
				TT_PANIC("Unable to seek to %d\n", p_src);
			}
		}
		return false;
	}
	
	fs::size_type len(p_len);
	if (s_file->read(p_dest, len) != len)
	{
		if (p_dummy == false)
		{
			if (s_handler != 0)
			{
				s_handler->handleError(CardError_Read);
			}
			else
			{
				TT_PANIC("Unable to read %d bytes.", p_len);
			}
		}
		return false;
	}
	
	// update internal copy of card
	if (p_dummy == false)
	{
		std::memcpy(s_mem + p_src, p_dest, static_cast<size_t>(p_len));
	}
	
	return true;
}


bool CardInterface::write(void* p_src, u32 p_dest, u32 p_len)
{
	FS_Trace("CardInterface::write: from %p to %d, %d bytes\n",
	         p_src, p_dest, p_len);
	
	if (p_dest >= s_size)
	{
		TT_PANIC("Destination %d out of bounds.", p_src);
		return false;
	}
	
	if (p_dest + p_len > s_size)
	{
		TT_PANIC("Destination + length (%d + %d = %d) out of bounds.",
		         p_dest, p_len, p_dest + p_len);
		return false;
	}
	
	if (p_src == 0)
	{
		TT_PANIC("Source must not be null.");
		return false;
	}
	
	// write page by page
	u32 pagesize  = 32;
	u32 firstpage = p_dest / pagesize;
	u32 lastpage  = (p_dest + p_len + pagesize - 1) / pagesize; // round up
	
	u32 begin  = 0;
	u32 end    = 0;
	
	u32 offset = p_dest % pagesize;
	u8* src  = reinterpret_cast<u8*>(p_src) - offset;
	u8* dst  = s_mem + (firstpage * pagesize);
	u32 todo = p_len;
	bool success = true;
	
	u32 written = 0;
	for (u32 page = firstpage; page <= lastpage; ++page)
	{
		// compare internal mem to p_src
		u32 size = pagesize - offset;
		if (size > todo)
		{
			size = todo;
		}
		if (page == lastpage || std::memcmp(src + offset, dst + offset, size) == 0)
		{
			// passed the last page or page is clean,
			// no copying or writing required
			if (begin != end)
			{
				// write pages marked as dirty
				if (success)
				{
					FS_Printf("Writing pages [%d - %d)\n", begin, end);
					u32 beginOffset  = begin * pagesize;
					u8* beginPointer = s_mem + beginOffset;
					u32 writeSize = (end - begin) * pagesize;
					written += (end - begin);
					
					fs::size_type toWrite(writeSize);
					
					if ((s_file->seek(static_cast<fs::off_type>(beginOffset), fs::SeekPos_Set) &&
					    s_file->write(beginPointer, toWrite) == toWrite) == false)
					{
						success = false;
					}
				}
			}
			// clear dirty range
			begin = page + 1;
			end   = page + 1;
		}
		else
		{
			// mark as dirty
			end = page + 1;
			FS_Printf("Page %d changed.\n", page);
			
			// update internal buffer
			std::memcpy(dst + offset, src + offset, size);
		}
		todo -= pagesize - offset;
		offset = 0;
		src += pagesize;
		dst += pagesize;
	}
	
	if (success == false)
	{
		if (s_handler != 0)
		{
			s_handler->handleError(CardError_Write);
		}
		else
		{
			TT_PANIC("Unable to write %d bytes.", p_len);
		}
		return false;
	}
	return true;
}


// Namespace end
}
}

