#include <algorithm>

#include <tt/code/Buffer.h>
#include <tt/fs/File.h>
#include <tt/fs/MemoryArchive.h>
#include <tt/fs/MemoryFileSystem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace fs {

MemoryFileSystem::Archives MemoryFileSystem::ms_archives;
bool                       MemoryFileSystem::ms_isInstantiated = false;
s32                        MemoryFileSystem::ms_id = -1;

struct MemInternal
{
	tt::fs::FilePtr passThroughFile;  // only set if using passthrough
	code::BufferPtr buffer;
	
	size_type length;
	pos_type  position;
	
	time_type creationTime;
	time_type accessTime;
	time_type writeTime;
	
	inline MemInternal()
	:
	passThroughFile(),
	buffer(),
	length(0),
	position(0),
	creationTime(0),
	accessTime(0),
	writeTime(0)
	{
	}
	
private:
	// No copying
	MemInternal(const MemInternal&);
	MemInternal& operator=(const MemInternal&);
};


// Public functions

FileSystemPtr MemoryFileSystem::instantiate(identifier p_identifier, identifier p_source)
{
	if (ms_isInstantiated)
	{
		TT_PANIC("Cannot register a MemoryFileSystem more than once");
		return FileSystemPtr();
	}
	
	FileSystemPtr filesys(new MemoryFileSystem(p_identifier, p_source));
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
	
	ms_isInstantiated = true;
	ms_id = p_identifier;
	
	return filesys;
}


MemoryFileSystem::~MemoryFileSystem()
{
}


// Basic file functions

bool MemoryFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
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
	
	MemoryArchive::File archiveFile;
	
	// MemoryFileSystem only works with read-only files
	if ((p_mode & (OpenMode_Write | OpenMode_Truncate | OpenMode_Append)) == 0)
	{
		for (Archives::iterator it = ms_archives.begin(); it != ms_archives.end(); ++it)
		{
			archiveFile = (*it)->getFile(p_path);
			if (archiveFile.content != 0)
			{
				//TT_Printf("MemoryFileSystem: ARCH open '%s'\n", p_path.c_str());
				break;
			}
		}
	}
	
	MemInternal* intern = new MemInternal;
	if (archiveFile.content == 0)
	{
		//TT_Printf("MemoryFileSystem: FILE open '%s'\n", p_path.c_str());
		// Not found in archives; use passthrough
		intern->passThroughFile = fs::open(p_path, p_mode, getSourceID());
		if (intern->passThroughFile == 0)
		{
			// Could not open file via source file system: file open failed
			// (no need to panic, since the other file system's open() implementation should already have done so)
			delete intern;
			return false;
		}
	}
	else
	{
		intern->buffer    = archiveFile.content;
		intern->length    = static_cast<size_type>(archiveFile.content->getSize());
		intern->position  = 0;
		intern->writeTime = archiveFile.writeTime;
		// TODO: fill in creation and access times
	}
	p_file->setData(reinterpret_cast<void*>(intern));
	return true;
}


bool MemoryFileSystem::close(File* p_file)
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
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		intern->passThroughFile.reset();
	}
	
	delete intern;
	p_file->setData(0);
	
	return true;
}


size_type MemoryFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
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
		return 0;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::read(intern->passThroughFile, p_buffer, p_length);
	}
	
	size_type start = static_cast<size_type>(intern->position);
	size_type end   = start + p_length;
	end = std::min(end, intern->length);
	size_type size = end - start;
	
	mem::copy8(p_buffer,
	           reinterpret_cast<const u8*>(intern->buffer->getData()) + intern->position,
	           static_cast<mem::size_type>(size));
	intern->position = static_cast<pos_type>(end);
	return size;
}


size_type MemoryFileSystem::readAsync(const FilePtr& p_file,
                                      void*          p_buffer,
                                      size_type      p_length,
                                      callback       p_callback,
                                      void*          p_arg)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::readAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


size_type MemoryFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		return 0;
	}
	
	if (p_length == 0)
	{
		return 0;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::write(intern->passThroughFile, p_buffer, p_length);
	}
	
	TT_PANIC("Cannot write to memory file '%s'. It is an in-memory file and therefore read-only.",
		p_file->getPath());
	
	return 0;
}


size_type MemoryFileSystem::writeAsync(const FilePtr& p_file,
                                       const void*    p_buffer,
                                       size_type      p_length,
                                       callback       p_callback,
                                       void*          p_arg)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::writeAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


bool MemoryFileSystem::flush(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::flush(intern->passThroughFile);
	}
	
	TT_PANIC("Cannot flush memory file '%s'. It is an in-memory file and therefore read-only.",
		p_file->getPath());
	
	return false;
}


// Content functions

code::BufferPtr MemoryFileSystem::getFileContent(const std::string& p_path)
{
	return FileSystem::getFileContent(p_path);
}


code::BufferPtr MemoryFileSystem::getFileContent(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return code::BufferPtr();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getFileContent(intern->passThroughFile);
	}
	
	return intern->buffer;
}


// Asynchronous status functions

bool MemoryFileSystem::isBusy(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::isBusy(intern->passThroughFile);
	}
	
	// Async I/O is not supported by MemoryFileSystem: ignore calls
	return false;
}


bool MemoryFileSystem::isSucceeded(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::isSucceeded(intern->passThroughFile);
	}
	
	// Async I/O is not supported by MemoryFileSystem: ignore calls
	return true;
}


bool MemoryFileSystem::wait(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::wait(intern->passThroughFile);
	}
	
	// Async I/O is not supported by MemoryFileSystem: ignore calls
	return true;
}


bool MemoryFileSystem::cancel(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::cancel(intern->passThroughFile);
	}
	
	// Async I/O is not supported by MemoryFileSystem: ignore calls
	return true;
}


// Position / size functions

bool MemoryFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::seek(intern->passThroughFile, p_offset, p_position);
	}
	
	pos_type offset = static_cast<pos_type>(p_offset);
	pos_type length = static_cast<pos_type>(intern->length);
	
	switch (p_position)
	{
	case SeekPos_Set:
		intern->position = std::max(pos_type(), std::min(offset, length));
		return true;
		
	case SeekPos_Cur:
		intern->position = std::max(pos_type(), std::min(intern->position + offset,
		                                                 length));
		return true;
		
	case SeekPos_End:
		intern->position = std::max(pos_type(), std::min(length + offset,
		                                                 length));
		return true;
	}
	return false;
}


bool MemoryFileSystem::seekToBegin(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::seekToBegin(intern->passThroughFile);
	}
	
	intern->position = 0;
	return true;
}


bool MemoryFileSystem::seekToEnd(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::seekToEnd(intern->passThroughFile);
	}
	
	intern->position = static_cast<pos_type>(intern->length);
	return true;
}


pos_type MemoryFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getPosition(intern->passThroughFile);
	}
	
	return intern->position;
}


size_type MemoryFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getLength(intern->passThroughFile);
	}
	
	return intern->length;
}


// Time functions

time_type MemoryFileSystem::getCreationTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getCreationTime(intern->passThroughFile);
	}
	
	return intern->creationTime;
}


time_type MemoryFileSystem::getAccessTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getAccessTime(intern->passThroughFile);
	}
	
	return intern->accessTime;
}


time_type MemoryFileSystem::getWriteTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::getWriteTime(intern->passThroughFile);
	}
	
	return intern->writeTime;
}


bool MemoryFileSystem::setWriteTime(const FilePtr& p_file, time_type p_time)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	MemInternal* intern = reinterpret_cast<MemInternal*>(p_file->getData());
	if (intern->passThroughFile != 0)
	{
		return fs::setWriteTime(intern->passThroughFile, p_time);
	}
	
	TT_PANIC("Cannot set write time for files in a memory archive.");
	return false;
}


// Create / Destroy functions

bool MemoryFileSystem::fileExists(const std::string& p_path)
{
	if (p_path.empty())
	{
		return false;
	}
	
	for (Archives::iterator it = ms_archives.begin(); it != ms_archives.end(); ++it)
	{
		if ((*it)->hasFile(p_path))
		{
			return true;
		}
	}
	
	// Not found in this FS; passthrough
	return fs::fileExists(p_path, getSourceID());
}


// Memory file/archive functions

bool MemoryFileSystem::addMemoryArchive(MemoryArchive* p_archive)
{
	ms_archives.push_back(p_archive);
	return true;
}


bool MemoryFileSystem::removeMemoryArchive(MemoryArchive* p_archive)
{
	Archives::iterator it = std::find(ms_archives.begin(),
	                                  ms_archives.end(),
	                                  p_archive);
	
	if (it == ms_archives.end())
	{
		TT_PANIC("Attempt to unregister unregistered archive.");
		return false;
	}
	
	ms_archives.erase(it);
	return true;
}


// Private functions

MemoryFileSystem::MemoryFileSystem(identifier p_id, identifier p_source)
:
PassThroughFileSystem(p_id, p_source)
{
}


// namespace end
}
}

