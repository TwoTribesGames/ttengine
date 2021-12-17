#include <algorithm>
#include <cstdio>

#include <tt/fs/File.h>
#include <tt/fs/CrcFileSystem.h>
#include <tt/math/hash/CRC32.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace fs {

static const size_type gs_headerSize = sizeof(u32) + sizeof(size_type);

struct CrcInternal
{
	inline CrcInternal()
	:
	file(),
	crc(),
	isReadMode(true),
	crcSize(0)
	{ }
	
	FilePtr                 file;       //!< Pointer to the file which is being CRC'd.
	tt::math::hash::CRC32   crc;        //!< Internal CRC for each file
	bool                    isReadMode; //!< Whether this file is opened in read or write mode
	size_type               crcSize;    //!< size of the data in the file that is crc'd (without the crc info)
	
private:
	// No copying
	CrcInternal(const CrcInternal&);
	CrcInternal& operator=(const CrcInternal&);
};


// Public functions

FileSystemPtr CrcFileSystem::instantiate(identifier p_identifier,
                                            identifier p_source)
{
	FileSystemPtr filesys(new CrcFileSystem(p_identifier, p_source));
	if (filesys == 0)
	{
		TT_PANIC("Failed to instantiate filesystem.");
		return filesys;
	}
	
	if (fs::registerFileSystem(filesys.get(), p_identifier) == false)
	{
		TT_PANIC("Failed to register filesytem.");
		filesys.reset();
	}
	
	return filesys;
}


bool CrcFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_file->getData() != 0)
	{
		TT_PANIC("File already open.");
		return false;
	}
	
	// Note: Maybe truncate works with the addition of write support. (Test first!)
	if ((p_mode & OpenMode_Truncate) != 0)
	{
		TT_PANIC("Write mode OpenMode_Truncate not available.");
		return false;
	}
	
	// Disable append. For crc to work the file should be written in one go
	if ((p_mode & OpenMode_Append) != 0)
	{
		TT_PANIC("Write mode OpenMode_Append not available.");
		return false;
	}
	
	// Check for mixed reading and writing. 
	if ((p_mode & (OpenMode_Read)) != 0 && (p_mode & OpenMode_Write) != 0)
	{
		TT_PANIC("CrcFileSystem supports either read or write. Not Both");
		return false;
	}
	typedef tt_ptr<CrcInternal>::unique CrcInternalAutoPtr;
	CrcInternalAutoPtr intern(new CrcInternal);
	intern->file = fs::open(p_path, p_mode, getSourceID());
	
	if (intern->file == 0)
	{
		TT_PANIC("Opening file '%s' Failed.", p_path.c_str());
		return false;
	}
	
	if ((p_mode & OpenMode_Read) != 0)
	{
		intern->isReadMode = true;
		bool success(true);
		
		// read the crc and filesize
		u32 readCrc = 0;
		success = success && fs::readInteger(intern->file, &readCrc);
		success = success && fs::readInteger(intern->file, &intern->crcSize);
		
		if(success == false)
		{
			TT_PANIC("Failed reading CRC size or header from file '%s'", p_path.c_str());
			return false;
		}
		else if (intern->crcSize <= 0)
		{
			TT_PANIC("Found incorrect crcSize: %d. CRC failed! '%s'", intern->crcSize, p_path.c_str());
			return false;
		}
		else if (intern->file->getLength() != gs_headerSize + intern->crcSize)
		{
			TT_PANIC("Found file length (%d) is not what was expect based on crcSize (%d). CRC failed! '%s'",
			         intern->file->getLength(), gs_headerSize + intern->crcSize, p_path.c_str());
			return false;
		}
		
		if (intern->crcSize == 0)
		{
			if (intern->file->getLength() != gs_headerSize)
			{
				TT_PANIC("Found file with crcSize 0, expect file size %d (headerSize), but found: %d. CRC failed! '%s'",
				         gs_headerSize, intern->file->getLength(), p_path.c_str());
				return false;
			}
			
			if (readCrc != 0)
			{
				TT_PANIC("CRC mismatch! Corrupted savedata or read/write is not in sync. '%s'", p_path.c_str());
				return false;
			}
		}
		else
		{
			// alocate buffer for whole file
			tt::code::Buffer crcBuffer(intern->crcSize);
			success = success && crcBuffer.getData() != 0;
			// read whole file
			success = success && (fs::read(intern->file, crcBuffer.getData(), crcBuffer.getSize()) == intern->crcSize);
			
			if(success == false)
			{
				TT_PANIC("Failed reading CRC from file '%s'", p_path.c_str());
				return false;
			}
			
			// create crc for the file and check with the read crc
			math::hash::CRC32 crc32;
			crc32.update(crcBuffer.getData(), static_cast<size_t>(crcBuffer.getSize()));
			success = success && crc32.getCRC() == readCrc;
			if(success == false)
			{
				TT_PANIC("CRC mismatch! Corrupted savedata or read/write is not in sync. '%s'", p_path.c_str());
				return false;
			}
		}
		
		// return to right after the crc info
		success = success && fs::seek(intern->file,
		                              gs_headerSize, 
		                              SeekPos_Set);
		
		if(success == false)
		{
			TT_PANIC("Failed reading crc from file '%s'", p_path.c_str());
			return false;
		}
	}
	else if ((p_mode & OpenMode_Write) != 0)
	{
		intern->isReadMode = false;
		
		bool success(true);
		success = success && fs::writeInteger(intern->file, intern->crc.getCRC());
		success = success && fs::writeInteger(intern->file, intern->crcSize);
		
		if(success == false)
		{
			TT_PANIC("Failed writing empty crc info to file. '%s'", p_path.c_str());
			return false;
		}
	}
	else
	{
		TT_PANIC("When using CrcFileSystem you should either write or read", p_path.c_str());
		return false;
	}
	
	p_file->setData(reinterpret_cast<void*>(intern.release()));
	return true;
}


bool CrcFileSystem::close(File* p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_WARN("File not open.");
		return false;
	}
	
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	
	if(intern->isReadMode == false)
	{
		bool success(true);
		
		success = success && fs::seekToBegin(intern->file);
		
		success = success && fs::writeInteger(intern->file, intern->crc.getCRC());
		success = success && fs::writeInteger(intern->file, intern->crcSize);
		
		if(success == false)
		{
			TT_PANIC("Failed writing empty crc info to file. '%s'", intern->file->getPath());
			return false;
		}
	}
	
	delete intern;
	p_file->setData(0);
	
	return true;
}


size_type CrcFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	FileSystem* src = getSource();
	
	if(src == 0)
	{
		return size_type();
	}
	
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	
	if(intern->isReadMode == false)
	{
		TT_PANIC("Can't Read from file opened in Write mode");
		return size_type();
	}
	
	// use source filesystem to read
	return src->read(intern->file, p_buffer, p_length);
}


size_type CrcFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	// update the internal crc
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	
	if(intern->isReadMode)
	{
		TT_PANIC("Can't Write from file opened in Read mode");
		return size_type();
	}
	
	intern->crc.update(p_buffer, static_cast<size_t>(p_length));
	intern->crcSize += p_length;
	
	// use source filesystem to write
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->write(intern->file, p_buffer, p_length);
}


bool CrcFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	
	if(intern->isReadMode == false)
	{
		TT_PANIC("CrcFileSystem::seek. Seek is disabled in the CrcFileSystem in Write mode");
		return false;
	}
	
	// use source filesystem to write 
	FileSystem* src = getSource();
	if(src == 0)
	{
		return false;
	}
	else
	{
		// only offset the the position when seeking from the begin 
		// not from the current position or the end (there is only an offset at the begin)
		if(p_position == SeekPos_Set)
		{
			p_offset += gs_headerSize;
		}
		return src->seek(intern->file, p_offset, p_position);
	}
}


pos_type CrcFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	return intern->file->getPosition() - gs_headerSize;
}


size_type CrcFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	CrcInternal* intern = reinterpret_cast<CrcInternal*>(p_file->getData());
	return intern->file->getLength() - gs_headerSize;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CrcFileSystem::CrcFileSystem(identifier p_id, identifier p_source)
:
PassThroughFileSystem(p_id, p_source)
{
}

// Namespace end
}
}
