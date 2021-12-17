#include <tt/savefs/TinySaveFS.h>
#include <tt/savefs/CardInterface.h>
#include <tt/fs/FS.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>
#include <tt/math/CRC32.h>
#include <tt/code/bufferutils.h>

#include <cstring>


//#define FS_DEBUG
#if !defined(TT_BUILD_FINAL) && defined(FS_DEBUG)
	#define FS_Printf TT_Printf
#else
	#define FS_Printf(...)
#endif

#define FS_WARN
#if !defined(TT_BUILD_FINAL) && defined(FS_WARN)
	#define FS_Warn TT_Printf
#else
	#define FS_Warn(...)
#endif

//#define FS_TRACE
#if !defined(TT_BUILD_FINAL) && defined(FS_TRACE)
	#define FS_Trace TT_Printf
#else
	#define FS_Trace(...)
#endif


namespace tt {
namespace savefs {

fs::FileSystem*   TinySaveFS::ms_fs           = 0;
u32               TinySaveFS::ms_type         = 0;
CardType          TinySaveFS::ms_cardType     = CardType_None;
u32               TinySaveFS::ms_cardSize     = 0;
u8*               TinySaveFS::ms_mem          = 0;
u32               TinySaveFS::ms_freeMem      = 0;
math::CRC32*      TinySaveFS::ms_crc          = 0;
TinySaveFS::Files TinySaveFS::ms_files;
bool              TinySaveFS::ms_autoFlush    = true;
bool              TinySaveFS::ms_flushEnabled = true;

using fs::File;


// Public functions

bool TinySaveFS::init(u32               p_type,
                      CardType          p_cardType,
                      CardErrorHandler* p_handler,
                      u32               p_baseFSType)
{
	FS_Trace("TinySaveFS::init: type: %d card: %d\n", p_type, p_cardType);
	
	if (ms_fs != 0)
	{
		TT_PANIC("already initialized.");
		return false;
	}
	
	// catches both invalid card types and CardType_None
	ms_cardSize = getCardSize(p_cardType);
	if (ms_cardSize == 0)
	{
		TT_PANIC("Card size may not be 0.");
		return false;
	}
	ms_cardType = p_cardType;
	
	FS_Printf("TinySaveFS::init: size: %d bytes (%p)\n", ms_cardSize, &ms_cardSize);
#ifdef TT_PLATFORM_NTR
	if (ms_cardSize > 65536)
	{
		// warn about memory usage
		// since the entire content of the card will be stored in memory
		FS_Warn("TinySaveFS::init: memory warning: card size exceeds 64KB\n");
	}
#endif
	
	ms_mem = new u8[ms_cardSize];
	if (ms_mem == 0)
	{
		TT_PANIC("Unable to allocate %d bytes for internal buffer.",
		         ms_cardSize);
		return false;
	}
	
	if (CardInterface::initialize(ms_cardType, p_handler, p_baseFSType) == false)
	{
		TT_PANIC("Unable to initialize card interface.");
		tt::code::helpers::safeDeleteArray(ms_mem);
		return false;
	}
	
	ms_crc = new math::CRC32;
	
	ms_fs = new fs::FileSystem;
	
	ms_fs->isBusy            = TinySaveFS::isBusy;
	ms_fs->isSucceeded       = TinySaveFS::isSucceeded;
	ms_fs->openFile          = TinySaveFS::openFile;
	ms_fs->closeFile         = TinySaveFS::closeFile;
	ms_fs->readFile          = TinySaveFS::readFile;
	ms_fs->readFileAsync     = TinySaveFS::readFile;
	ms_fs->writeFile         = TinySaveFS::writeFile;
	ms_fs->writeFileAsync    = TinySaveFS::writeFile;
	ms_fs->flush             = TinySaveFS::flush;
	ms_fs->cancelFile        = TinySaveFS::cancelFile;
	ms_fs->waitAsync         = TinySaveFS::waitAsync;
	ms_fs->seekFile          = TinySaveFS::seekFile;
	ms_fs->getLength         = TinySaveFS::getLength;
	ms_fs->getPosition       = TinySaveFS::getPosition;
	ms_fs->fileExists        = TinySaveFS::fileExists;
	ms_fs->getCreationTime   = TinySaveFS::getCreationTime;
	ms_fs->getLastAccessTime = TinySaveFS::getLastAccessTime;
	ms_fs->getLastWriteTime  = TinySaveFS::getLastWriteTime;
	
	if (fs::FS::registerFileSystem(p_type, ms_fs) == false)
	{
		TT_PANIC("Unable to register filesystem.");
		
		CardInterface::end();
		tt::code::helpers::safeDeleteArray(ms_mem);
		tt::code::helpers::safeDelete(ms_crc);
		tt::code::helpers::safeDelete(ms_fs);
		return false;
	}
	
	ms_type = p_type;
	
	// now load card content
	if (CardInterface::read(0, ms_mem, ms_cardSize) == false)
	{
		TT_PANIC("Unable to read data from card.");
		end();
		return false;
	}
	
	ms_freeMem = ms_cardSize;
	
	// read file table
	const u8* scratch = ms_mem;
	size_t    remain  = ms_freeMem;
	u32 crc           = code::bufferutils::get<u32>(scratch, remain);
	
	FS_Printf("TinySaveFS::init: Header crc 0x%08X.\n", crc);
	
	u32 filecount = code::bufferutils::get<u32>(scratch, remain);
	FS_Printf("TinySaveFS::init: %d files found.\n", filecount);
	
	// calculate crc from file headers
	u32 headersize = sizeof(u32) + (filecount* 4 * sizeof(u32));
	FS_Printf("TinySaveFS::init: header %d bytes.\n", headersize);
	
	if (headersize >= (ms_cardSize - sizeof(u32)))
	{
		// headersize is larger than card minus crc
		// wipe card
		TT_WARN("Invalid amount of files (%d), wiping card.", filecount);
		// clears crc and file count
		std::memset(ms_mem, 0, sizeof(u32) * 2);
		crc = 0;
		headersize = sizeof(u32); // file count only
		filecount = 0;
	}
	
	u32 headercrc = ms_crc->calcCRC(ms_mem + sizeof(u32), headersize);
	if (headercrc != crc)
	{
		TT_WARN("Header crc (0x%08X) does not match stored crc (0x%08X).",
		        headercrc, crc);
		// wipe card
		// clears crc and file count
		std::memset(ms_mem, 0, sizeof(u32) * 2);
		crc = 0;
		headersize = sizeof(u32); // file count only
		filecount = 0;
	}
	
	bool remap = false;
	for (u32 i = 0; i < filecount; ++i)
	{
		// read file table entry
		FileHeader header;
		header.nameCrc = code::bufferutils::get<u32>(scratch, remain);
		header.dataCrc = code::bufferutils::get<u32>(scratch, remain);
		header.size    = code::bufferutils::get<u32>(scratch, remain);
		header.start   = code::bufferutils::get<u32>(scratch, remain);
		header.newsize = header.size;
		
		FS_Printf("TinySaveFS::init: File %p (crc %p) size: %d start: %d\n",
		          header.nameCrc, header.dataCrc, header.size, header.start);
		
		// check if data is okay
		u32 crc = ms_crc->calcCRC(ms_mem + header.start, header.size);
		if (crc != header.dataCrc)
		{
			FS_Warn("TinySaveFS::init: File %p: data crc mismatch "
			        "(found %p, expected %p).\n",
			        header.nameCrc, crc, header.dataCrc);
			
			// whipe file
			header.newsize = 0;
			header.dataCrc = 0;
			remap = true;
		}
		
		// check if file already exists
		bool duplicate = false;
		for (Files::iterator it = ms_files.begin(); it != ms_files.end(); ++it)
		{
			if ((*it).nameCrc == header.nameCrc)
			{
				FS_Warn("TinySaveFS::init: found duplicate file (%p).\n",
				        header.nameCrc);
				duplicate = true;
				remap = true;
				break;
			}
		}
		
		if (duplicate == false)
		{
			ms_files.push_back(header);
			remain -= header.newsize;
		}
	}
	
	if (remap)
	{
		remapFiles();
	}
	
	ms_freeMem = remain;
	FS_Printf("TinySaveFS::init: %d free card space left.\n", ms_freeMem);
	FS_Printf("TinySaveFS::init: successful.\n");
	return true;
}


bool TinySaveFS::end()
{
	FS_Trace("TinySaveFS::end\n");
	
	if (ms_fs == 0)
	{
		TT_PANIC("fs already ended or never initialized.");
		return false;
	}
	
	// close all open files and clean up internal structures
	
	if (CardInterface::end() == false)
	{
		TT_PANIC("Unable to end card interface.");
		return false;
	}
	
	if (fs::FS::unregisterFileSystem(ms_type) == false)
	{
		TT_PANIC("Unable to unregister filesystem.");
		return false;
	}
	tt::code::helpers::safeDeleteArray(ms_mem);
	tt::code::helpers::safeDelete(ms_fs);
	tt::code::helpers::safeDelete(ms_crc);
	
	FS_Printf("TinySaveFS::end: successful.\n");
	return true;
}


void TinySaveFS::enableFlush(bool p_enable)
{
	ms_flushEnabled = p_enable;
}


void TinySaveFS::setAutoFlush(bool p_enable)
{
	ms_autoFlush = p_enable;
}


bool TinySaveFS::flush()
{
	if (ms_flushEnabled == false)
	{
		return true;
	}
	
	if (CardInterface::write(ms_mem, 0, ms_cardSize) == false)
	{
		TT_PANIC("Error writing to card\n");
		return false;
	}
	return true;
}


// Private Functions

bool TinySaveFS::isBusy(volatile const File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	return false;
}


bool TinySaveFS::isSucceeded(volatile const File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	return true;
}


bool TinySaveFS::openFile(File* p_file, const char* p_path, const char* p_mode)
{
	FS_Trace("TinySaveFS::openFile: %s (%s)\n", p_path, p_mode);
	
	if (p_file->handle != 0)
	{
		if (closeFile(p_file) == false)
		{
			TT_PANIC("Unable to close opened file.");
			return false;
		}
	}
	
	// parse open mode
	bool read   = false;
	bool write  = false;
	bool append = false;
	bool create = false;
	
	for (int i = 0; p_mode[i] != '\0'; ++i)
	{
		switch (p_mode[i])
		{
		case 'r':
			if (write || append)
			{
				TT_PANIC("Cannot combine 'r' with other mode ('%s').", p_mode);
				return false;
			}
			read = true;
			break;
			
		case 'w':
			if (read || append)
			{
				TT_PANIC("Cannot combine 'w' with other mode ('%s').", p_mode);
				return false;
			}
			write = true;
			create = true;
			break;
			
		case 'a':
			if (read || write)
			{
				TT_PANIC("Cannot combine 'a' with other mode ('%s').", p_mode);
				return false;
			}
			append = true;
			write  = true;
			break;
			
		case '+':
			if (read == false && write == false && append == false)
			{
				TT_PANIC("Missing specifier before + in mode '%s'.", p_mode);
				return false;
			}
			read  = true;
			write = true;
			break;
			
		case 'b': // binary mode, ignored
		case 't': // text(translated) mode, ignored
			break;
			
		default:
			TT_PANIC("Unknown identifier '%c' in mode '%s'.",
			         p_mode[i], p_mode);
			return false;
		}
	}
	
	if (read == false && write == false)
	{
		TT_PANIC("Invalid open mode '%s'.", p_mode);
		return false;
	}
	
	// get crc of file
	u32 crc = getCRC(p_path);
	
	// see if file exists
	Files::iterator header;
	for (header = ms_files.begin(); header != ms_files.end(); ++header)
	{
		if ((*header).nameCrc == crc)
		{
			FS_Printf("TinySaveFS::openFile: '%s' exists.\n", p_path);
			break;
		}
	}
	if (header == ms_files.end())
	{
		FS_Printf("TinySaveFS::openFile: '%s' does not exist.\n", p_path);
		
		if (create == false)
		{
			TT_WARN("'%s' does not exist.", p_path);
			return false;
		}
		
		// otherwise, create it
		FileHeader head;
		head.nameCrc  = crc;
		head.dataCrc  = 0;
		head.size     = 0;
		head.newsize  = 0;
		head.start    = 0;
		head.newstart = 0;
		if (ms_freeMem < (4 * sizeof(u32)))
		{
			TT_PANIC("Unable to create new file '%s': out of space.");
			return false;
		}
		// decrease amount of free card space
		ms_freeMem -= 4 * sizeof(u32);
		
		header = ms_files.insert(ms_files.end(), head);
		// remap files (not saved untill all is flushed)
		remapFiles();
	}
	
	// check if the open mode is possible
	if (read)
	{
		// no open file handles in write mode are allowed
		for (FileHandles::iterator it = (*header).files.begin();
		     it != (*header).files.end(); ++it)
		{
			if ((*it).write)
			{
				// someone opened this file in write mode
				TT_PANIC("Unable to open file '%s'.", p_path);
				return false;
			}
		}
	}
	if (write)
	{
		// no other file handles are allowed
		if ((*header).files.empty() == false)
		{
			TT_PANIC("Unable to open file '%s'.", p_path);
			return false;
		}
	}
	
	// create a new file handle;
	FileHandle handle;
	handle.info  = &(*header);
	handle.read  = read;
	handle.write = write;
	// fill in the rest of the information according to the open mode
	if (create == false)
	{
		handle.size = (*header).size;
		if (append)
		{
			handle.pointer = handle.size;
		}
		else
		{
			handle.pointer = 0;
		}
		
		if (write)
		{
			// create a new memory block to hold the content of the file
			u8* data = new u8[handle.size];
			if (data == 0)
			{
				TT_PANIC("Unable to allocate %d bytes to hold file.",
				         handle.size);
				return false;
			}
			std::memcpy(data, ms_mem + (*header).start, handle.size);
			
			MemNode node;
			node.data  = data;
			node.size  = handle.size;
			node.start = 0;
			handle.data.push_back(node);
		}
		else
		{
			// memory node points directly to data
			MemNode node;
			node.data  = ms_mem + (*header).start;
			node.size  = handle.size;
			node.start = 0;
			handle.data.push_back(node);
		}
	}
	else
	{
		handle.size    = 0;
		handle.pointer = 0;
		// data can stay empty since there's no data yet
		
		// increase amount of free mem
		ms_freeMem += (*header).size;
		
		// file will be destroyed during next remapping
		// otherwise remapping could run out of card space
		(*header).newsize = 0;
	}
	
	// insert handle in file's handle list
	FileHandles::iterator it =
		(*header).files.insert((*header).files.end(), handle);
	
	FS_Printf("TinySaveFS::openFile: pointer at %d\n", handle.pointer);
	
	// set pointer to file handle
	p_file->handle = static_cast<void*>(&(*it));
	return true;
}


bool TinySaveFS::closeFile(File* p_file)
{
	FS_Trace("TinySaveFS::closeFile\n");
	
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	// close file
	FileHandle* handle = reinterpret_cast<FileHandle*>(p_file->handle);
	if (handle->write == false)
	{
		// read only, no writing needs to be done, just close the handle
		FileHeader* header = handle->info;
		for (FileHandles::iterator it = header->files.begin();
		     it != header->files.end(); ++it)
		{
			if (&(*it) == handle)
			{
				header->files.erase(it);
				p_file->handle = 0;
				return true;
			}
		}
		// file handle not found.
		TT_PANIC("Unknown file handle.");
		return false;
	}
	
	// update file crc
	u32 remaining = handle->size;
	math::CRC32 crc;
	for (MemList::iterator it = handle->data.begin();
	     it != handle->data.end(); ++it)
	{
		u32 todo = std::min(remaining, (*it).size);
		crc.update((*it).data, todo);
		remaining -= todo;
	}
	handle->info->dataCrc = crc.getHash();
	
	// set new size
	handle->info->newsize = handle->size;
	
	// remap
	remapFiles();
	
	// write file data
	u32 start   = handle->info->start;
	u32 towrite = handle->info->size;
	for (MemList::iterator it = handle->data.begin();
	     it != handle->data.end(); ++it)
	{
		u32 todo = std::min(towrite, (*it).size);
		std::memcpy(ms_mem + start, (*it).data, todo);
		towrite -= todo;
		start   += todo;
		
		// free memory
		delete[] (*it).data;
	}
	if (towrite != 0)
	{
		// something went terribly wrong
		TT_PANIC("Internal FileSystem error, FS instable.");
		
		// attempt some recovery...
		return false;
	}
	
	// write to card
	if (ms_autoFlush)
	{
		if (flush() == false)
		{
			TT_PANIC("Writing to card failed.");
			return false;
		}
	}
	
	
	// close handle
	FileHeader* header = handle->info;
	for (FileHandles::iterator it = header->files.begin();
	     it != header->files.end(); ++it)
	{
		if (&(*it) == handle)
		{
			header->files.erase(it);
			p_file->handle = 0;
			
			return true;
		}
	}
	
	// file handle not found.
	TT_PANIC("Unknown file handle.");
	return false;
}


s32 TinySaveFS::readFile(File* p_file, void* p_dst, s32 p_len)
{
	FS_Trace("TinySaveFS::readFile: %d bytes\n", p_len);
	
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return -1;
	}
	
	FileHandle* handle = reinterpret_cast<FileHandle*>(p_file->handle);
	
	if (handle->read == false)
	{
		TT_PANIC("File not opened in read mode.");
		return -1;
	}
	
	if (handle->pointer == handle->size)
	{
		// EOF
		return 0;
	}
	
	// find current data block
	MemList::iterator mem;
	for (mem = handle->data.begin(); mem != handle->data.end(); ++mem)
	{
		if (((*mem).start + (*mem).size) > handle->pointer)
		{
			// pointer is located in this memory block
			break;
		}
	}
	
	u8* dest = reinterpret_cast<u8*>(p_dst);
	s32 read = 0;
	while (p_len > 0)
	{
		if (mem != handle->data.end() &&
		    handle->pointer == ((*mem).size + (*mem).start))
		{
			++mem;
		}
		if (mem == handle->data.end())
		{
			// EOF
			break;
		}
		
		u32 offset = handle->pointer - (*mem).start; // offset within mem block
		s32 todo = std::min(p_len, s32((*mem).size - offset));
		
		// copy
		std::memcpy(dest, (*mem).data + offset, size_t(todo));
		read  += todo;
		dest  += todo;
		p_len -= todo;
		
		// update file pointer
		handle->pointer += u32(todo);
	}
	
	return read;
}


s32 TinySaveFS::writeFile(File* p_file, const void* p_src, s32 p_len)
{
	FS_Trace("TinySaveFS::writeFile: %d bytes\n", p_len);
	
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return -1;
	}
	
	FileHandle* handle = reinterpret_cast<FileHandle*>(p_file->handle);
	
	if (handle->write == false)
	{
		TT_PANIC("File not opened in write mode.");
		return -1;
	}
	
	// find current data block
	MemList::iterator mem;
	for (mem = handle->data.begin(); mem != handle->data.end(); ++mem)
	{
		if (((*mem).start + (*mem).size) < handle->pointer &&
		   (*mem).start >= handle->pointer)
		{
			// pointer is located in this memory block
			break;
		}
	}
	
	s32 written   = 0;
	const u8* src = reinterpret_cast<const u8*>(p_src);
	while (p_len > 0)
	{
		if (mem != handle->data.end() &&
		    handle->pointer == ((*mem).size + (*mem).start))
		{
			++mem;
		}
		
		if (mem == handle->data.end())
		{
			// allocate more mem
			// create new memory block
			s32 toAlloc = std::min(p_len, s32(ms_freeMem));
			if (toAlloc == 0)
			{
				break;
			}
			
			u8* memblock = new u8[toAlloc];
			if (memblock == 0)
			{
				TT_PANIC("Unable to allocate %d bytes.", memblock);
				break;
			}
			
			MemNode node;
			node.data  = memblock;
			node.size  = u32(toAlloc);
			node.start = handle->pointer;
			
			mem = handle->data.insert(handle->data.end(), node);
			
			ms_freeMem -=u32(toAlloc);
		}
		
		u32 offset = handle->pointer - (*mem).start;
		s32 todo   = std::min(p_len, s32((*mem).size - offset));
		
		// copy
		std::memcpy((*mem).data + offset, src, size_t(todo));
		written += todo;
		src     += todo;
		p_len   -= todo;
		
		// update file pointer
		handle->pointer += u32(todo);
		if (handle->pointer > handle->size)
		{
			// file has grown
			handle->size = handle->pointer;
		}
	}
	return written;
}


bool TinySaveFS::flush(File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	return true;
}


void TinySaveFS::cancelFile(File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return;
	}
}


bool TinySaveFS::waitAsync(File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	return true;
}


bool TinySaveFS::seekFile(File* p_file, s32 p_offset, int p_origin)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	FileHandle* handle = reinterpret_cast<FileHandle*>(p_file->handle);
	s32 currentpos = s32(handle->pointer);
	
	switch (p_origin)
	{
	case SEEK_SET: currentpos  = p_offset; break;
	case SEEK_CUR: currentpos += p_offset; break;
	case SEEK_END: currentpos  = s32(handle->size) + p_offset; break;
	}
	
	currentpos = std::max(currentpos, s32(0));
	currentpos = std::min(currentpos, s32(handle->size));
	handle->pointer = u32(currentpos);
	
	return true;
}


u32 TinySaveFS::getLength(File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return 0;
	}
	
	return reinterpret_cast<FileHandle*>(p_file->handle)->size;
}


u32 TinySaveFS::getPosition(File* p_file)
{
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return 0;
	}
	
	return reinterpret_cast<FileHandle*>(p_file->handle)->pointer;
}


bool TinySaveFS::fileExists(const char* p_path)
{
	FS_Printf("TinySaveFS::fileExists: '%s'\n", p_path);
	
	u32 crc = getCRC(p_path);
	for (Files::const_iterator it = ms_files.begin();
	     it != ms_files.end();
	     ++it)
	{
		if (crc == (*it).nameCrc)
		{
			return true;
		}
	}
	
	return false;
}


u64 TinySaveFS::getCreationTime(fs::File* p_file)
{
	FS_Trace("TinySaveFS::getLastAccessTime\n");
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return 0;
	}
	
	return 0;
}


u64 TinySaveFS::getLastWriteTime(fs::File* p_file)
{
	FS_Trace("TinySaveFS::getLastAccessTime\n");
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return 0;
	}
	
	return 0;
}


u64 TinySaveFS::getLastAccessTime(fs::File* p_file)
{
	FS_Trace("TinySaveFS::getLastAccessTime\n");
	if (p_file->handle == 0)
	{
		TT_PANIC("File not open.");
		return 0;
	}
	
	return 0;
}


u32 TinySaveFS::getCRC(const char* p_str)
{
	if (p_str == 0)
	{
		TT_PANIC("String must not be 0.");
		return 0;
	}
	
	u32 len = std::strlen(p_str);
	return ms_crc->calcCRC(p_str, len);
}


void TinySaveFS::remapFiles()
{
	FS_Trace("TinySaveFS::remapFiles\n");
	
	u32 start = 0;
	start += sizeof(u32) * 2; // crc + filecount
	start += ms_files.size() * (4 * sizeof(u32));
	
	// calculate new start positions of all files
	for (Files::iterator it = ms_files.begin(); it != ms_files.end(); ++it)
	{
		if ((*it).newsize != 0)
		{
			(*it).newstart = start;
			start += (*it).newsize;
		}
		else
		{
			(*it).newstart = 0;
		}
		FS_Printf("TinySaveFS::remapFiles: %08X (%08X) size %d->%d, start %d->%d\n",
		          (*it).nameCrc,
		          (*it).dataCrc,
		          (*it).size, (*it).newsize,
		          (*it).start, (*it).newstart);
	}
	
	// move files to front of memory where possible
	for (Files::iterator it = ms_files.begin(); it != ms_files.end(); ++it)
	{
		if ((*it).newsize == 0)
		{
			continue;
		}
		
		if ((*it).newstart > (*it).start)
		{
			// file needs to be moved to back of memory
			Files::iterator it2 = it;
			while (it2 != ms_files.end())
			{
				if ((*it2).newstart <= (*it2).start)
				{
					// one past last one
					break;
				}
				++it2;
			}
			
			do
			{
				--it2;
				u32 size = std::min((*it2).newsize, (*it2).size);
				if (size > 0)
				{
					std::memmove(ms_mem + (*it2).newstart,
					             ms_mem + (*it2).start,
					             size);
				}
				(*it2).start = (*it2).newstart;
				(*it2).size  = (*it2).newsize;
				
				// update all file's open read only handles
				for (FileHandles::iterator handle = (*it2).files.begin();
				     handle != (*it2).files.end(); ++handle)
				{
					if ((*handle).write == false &&
					    (*handle).data.empty() == false)
					{
						(*(*handle).data.begin()).data = ms_mem + (*it2).start;
					}
				}
			}
			while (it2 != it);
		}
		
		if ((*it).newstart == (*it).start)
		{
			// nothing needs to be moved
			
			(*it).size = (*it).newsize;
			continue;
		}
		
		// copy file
		u32 size = std::min((*it).newsize, (*it).size);
		if (size > 0)
		{
			std::memmove(ms_mem + (*it).newstart,
			             ms_mem + (*it).start,
			             size);
		}
		(*it).start = (*it).newstart;
		(*it).size  = (*it).newsize;
		
		// update all file's open read only handles
		for (FileHandles::iterator handle = (*it).files.begin();
		     handle != (*it).files.end(); ++handle)
		{
			if ((*handle).write == false &&
			    (*handle).data.empty() == false)
			{
				(*(*handle).data.begin()).data = ms_mem + (*it).start;
			}
		}
	}
	
	// write file table
	u8* scratch   = ms_mem + sizeof(u32); // skip past header crc
	size_t remain = ms_cardSize - sizeof(u32);
	
	u32 filecount = ms_files.size();
	code::bufferutils::put(filecount, scratch, remain);
	FS_Printf("TinySaveFS::remapFiles: %d files.\n", filecount);
	for (Files::iterator it = ms_files.begin(); it != ms_files.end(); ++it)
	{
		code::bufferutils::put((*it).nameCrc, scratch, remain);
		code::bufferutils::put((*it).dataCrc, scratch, remain);
		code::bufferutils::put((*it).size, scratch, remain);
		code::bufferutils::put((*it).start, scratch, remain);
	}
	
	// update header crc
	u32 headersize = sizeof(u32) + (4 * sizeof(u32) * filecount);
	FS_Printf("TinySaveFS::remapFiles: header %d bytes.\n", headersize);
	
	u32 crc = ms_crc->calcCRC(ms_mem + sizeof(u32), headersize);
	scratch = ms_mem;
	remain  = ms_cardSize;
	code::bufferutils::put(crc, scratch, remain);
	FS_Printf("TinySaveFS::remapFiles: new header crc: 0x%08X.\n", crc);
}


// Namespace end
}
}

