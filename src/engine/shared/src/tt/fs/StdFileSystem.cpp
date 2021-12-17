#include <cstdio>

#include <tt/fs/File.h>
#include <tt/fs/StdFileSystem.h>


namespace tt {
namespace fs {

struct StdFsInternal
{
	std::FILE* handle;
	
	inline StdFsInternal()
	:
	handle(0)
	{
	}
	
	inline ~StdFsInternal()
	{
		if (handle != 0)
		{
			TT_PANIC("File still open.");
			if (std::fclose(handle) != 0)
			{
				TT_PANIC("Failed to close file.");
			}
		}
	}
	
private:
	// No copying
	StdFsInternal(const StdFsInternal&);
	StdFsInternal& operator=(const StdFsInternal&);
};


// Public functions

FileSystemPtr StdFileSystem::instantiate(fs::identifier p_identifier)
{
	FileSystemPtr filesys(new StdFileSystem(p_identifier));
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


StdFileSystem::~StdFileSystem()
{
}


// Feature support check functions

bool StdFileSystem::supportsSaving()
{
	return true;
}


bool StdFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
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
	
	// parse open mode
	const char* openmode = 0;
	
	switch (p_mode & (~OpenMode_AtEnd))
	{
	case OpenMode_Read:
		openmode = "rb";
		break;
		
	case OpenMode_Write:
	case OpenMode_Write | OpenMode_Truncate:
		openmode = "wb";
		break;
		
	case OpenMode_Write | OpenMode_Append:
		openmode = "ab";
		break;
		
	case OpenMode_Read | OpenMode_Write:
	case OpenMode_Read | OpenMode_Write | OpenMode_Append:
		openmode = "r+b";
		break;
		
	case OpenMode_Read | OpenMode_Write | OpenMode_Truncate:
		openmode = "w+b";
		break;
		
	default:
		TT_PANIC("Unsupported open mode.");
		return false;
	}
	
	std::FILE* file = std::fopen(p_path.c_str(), openmode);
	
	if (file == NULL)
	{
		TT_PANIC("Unable to open %s.", p_path.c_str());
		return false;
	}
	
	StdFsInternal* intern = new StdFsInternal;
	intern->handle = file;
	
	p_file->setData(reinterpret_cast<void*>(intern));
	return true;
}


bool StdFileSystem::close(File* p_file)
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
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	if (intern->handle == NULL)
	{
		TT_PANIC("Attempt to close invalid file.");
		return false;
	}
	
	bool success = true;
	if (std::fclose(intern->handle) != 0)
	{
		TT_PANIC("Failed to close file.");
		success = false;
		// we'll need to clean up the internal data to prevent more leaks.
	}
	intern->handle = 0;
	
	delete intern;
	p_file->setData(0);
	
	return success;
}


size_type StdFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
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
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	if (intern->handle == NULL)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	const size_t nNumberOfBytesToRead = static_cast<size_t>(p_length);
	const size_t nNumberOfBytesRead = std::fread(p_buffer, 1, nNumberOfBytesToRead, intern->handle);
	if (ferror(intern->handle) != 0)
	{
		TT_PANIC("Error reading from file.");
		return size_type();
	}
	
	size_type ret = static_cast<size_type>(nNumberOfBytesRead);
	return ret;
}


size_type StdFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
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
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	if (intern->handle == NULL)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	const size_t nNumberOfBytesToWrite = static_cast<size_t>(p_length);
	const size_t nNumberOfBytesWritten = std::fwrite(p_buffer, 1, nNumberOfBytesToWrite, intern->handle);
	if (ferror(intern->handle) != 0)
	{
		TT_PANIC("Error writing to file.");
		return size_type();
	}
	
	size_type ret = static_cast<size_type>(nNumberOfBytesWritten);
	return ret;
}


bool StdFileSystem::flush(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	if (std::fflush(intern->handle) != 0)
	{
		TT_PANIC("Flushing failed.");
		return false;
	}
	return true;
}


bool StdFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	long offset = static_cast<long>(p_offset);
	int origin;
	switch (p_position)
	{
	case SeekPos_Set: origin = SEEK_SET; break;
	case SeekPos_Cur: origin = SEEK_CUR; break;
	case SeekPos_End: origin = SEEK_END; break;
	default:
		TT_PANIC("Unknown seek mode.");
		return false;
	}
	
	if (std::fseek(intern->handle, offset, origin) != 0)
	{
		TT_PANIC("Error while seeking.");
		return false;
	}
	
	return true;
}


pos_type StdFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	
	long position = std::ftell(intern->handle);
	if (position == -1)
	{
		TT_PANIC("Error getting file pointer.");
		return pos_type();
	}
	
	return static_cast<pos_type>(position);
}


size_type StdFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	StdFsInternal* intern = reinterpret_cast<StdFsInternal*>(p_file->getData());
	
	std::fpos_t pos;
	std::fgetpos(intern->handle, &pos);
	std::fseek(intern->handle, 0, SEEK_END);
	long size = std::ftell(intern->handle);
	std::fsetpos(intern->handle, &pos);
	
	return static_cast<size_type>(size);
}


bool StdFileSystem::fileExists(const std::string& p_path)
{
	if (p_path.empty())
	{
		return false;
	}
	
	std::FILE* file = std::fopen(p_path.c_str(), "rb");
	if (file == NULL)
	{
		return false;
	}
	std::fclose(file);
	
	return true;
}


// Private functions

StdFileSystem::StdFileSystem(fs::identifier p_id)
:
FileSystem(p_id)
{
	
}


// namespace end
}
}
