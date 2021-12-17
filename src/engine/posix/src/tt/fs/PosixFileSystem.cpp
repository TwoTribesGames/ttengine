#include <fstream>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/File.h>
#include <tt/fs/PosixFileSystem.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_printf.h>

#ifdef TT_PLATFORM_SDL
#include <SDL2/SDL_filesystem.h>
#endif

#ifdef TT_PLATFORM_OSX_MAC
namespace tt {
	namespace system {
		std::string getOldMacOSXSavePath();
	}
}
#endif


namespace tt {
namespace fs {

struct PosixFsInternal
{
	int fileDescriptor;
	
	
	inline PosixFsInternal()
	:
	fileDescriptor(-1)
	{
	}
	
	inline ~PosixFsInternal()
	{
		if (fileDescriptor != -1)
		{
			TT_PANIC("File still open.");
			if (::close(fileDescriptor) != 0)
			{
				TT_PANIC("Failed to close file (error %d): %s", errno, std::strerror(errno));
			}
		}
	}
	
private:
	// No copying
	PosixFsInternal(const PosixFsInternal&);
	PosixFsInternal& operator=(const PosixFsInternal&);
};

struct DirInternal
{
	DIR*        handle;
	bool        ignoreHidden;
	std::string filter;
	std::string searchPath;
	
	
	inline DirInternal()
	:
	handle(0),
	ignoreHidden(false),
	filter(),
	searchPath()
	{ }
	
private:
	// No copying
	DirInternal(const DirInternal&);
	DirInternal& operator=(const DirInternal&);
};


bool getFileStats(const FilePtr& p_file, struct stat* p_fileInfo)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	// Attempt to retrieve the status of the specified file
	if (::fstat(intern->fileDescriptor, p_fileInfo) != 0)
	{
		TT_PANIC("Retrieving information about file failed with error %d: %s",
		         errno, std::strerror(errno));
		return false;
	}
	
	return true;
}

// Seconds difference between 1601-01-01 00:00:00 UTC and 1970-01-01 00:00:00 UTC
static const uint64_t EPOCH_DIFFERENCE_SECONDS = 11644473600ull;
// Filetime is number of 100 nsec units.. (one more 10x than than usec)
static const uint64_t SECONDS_TO_FILETIME = 10000000ull;
static const uint64_t NSEC_TO_FILETIME = (1 / 100ull);
static const uint64_t USEC_TO_FILETIME = 10ull;

time_type createTimeType(time_t unixtime)
{
	return (unixtime + EPOCH_DIFFERENCE_SECONDS) * SECONDS_TO_FILETIME;
}

time_type createTimeType(const struct timespec& ts)
{
	uint64_t total_ft = (ts.tv_sec + EPOCH_DIFFERENCE_SECONDS) * SECONDS_TO_FILETIME;
	total_ft += (ts.tv_nsec * NSEC_TO_FILETIME);

	return total_ft;
}

time_type createTimeType(const struct timeval& tv)
{
	uint64_t total_ft = (tv.tv_sec + EPOCH_DIFFERENCE_SECONDS ) * SECONDS_TO_FILETIME;
	total_ft += tv.tv_usec * USEC_TO_FILETIME;

	return total_ft;
}

void convertTimeType(struct timeval& tv, time_type p_time)
{
	tv.tv_sec = (p_time / SECONDS_TO_FILETIME) - EPOCH_DIFFERENCE_SECONDS;
	tv.tv_usec = (p_time % SECONDS_TO_FILETIME) / USEC_TO_FILETIME;
}

//--------------------------------------------------------------------------------------------------
// Public member functions

FileSystemPtr PosixFileSystem::instantiate(fs::identifier p_identifier, const std::string& p_appName)
{
	FileSystemPtr filesys(new PosixFileSystem(p_identifier, p_appName));
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


PosixFileSystem::~PosixFileSystem()
{
}


// ========== Feature support check functions ==========

bool PosixFileSystem::supportsSaving()
{
	return true;
}


bool PosixFileSystem::supportsDirectories()
{
	return true;
}


// ========== Basic file functions ==========

bool PosixFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
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
	
	// Parse open mode
	int openmode = 0;
	switch (p_mode & (~OpenMode_AtEnd))
	{
	case OpenMode_Read:
		openmode |= O_RDONLY;
		break;
		
	case OpenMode_Write:
	case OpenMode_Write | OpenMode_Truncate:
		openmode |= O_WRONLY | O_CREAT | O_TRUNC;
		break;
		
	case OpenMode_Append:
		openmode |= O_WRONLY | O_APPEND;
		break;
		
	case OpenMode_Write | OpenMode_Append:
		openmode |= O_WRONLY | O_CREAT | O_APPEND;
		break;
		
	case OpenMode_Read | OpenMode_Write:
	case OpenMode_Read | OpenMode_Write | OpenMode_Append:
		openmode |= O_RDWR | O_CREAT | O_APPEND;
		break;
		
	case OpenMode_Read | OpenMode_Write | OpenMode_Truncate:
		openmode |= O_RDWR | O_CREAT | O_TRUNC;
		break;
		
	default:
		TT_PANIC("Unsupported open mode (0x%08X). Notify daniel@twotribes.com of this issue.",
		         p_mode & (~OpenMode_AtEnd));
		return false;
	}
	
	int fileDescriptor = ::open(p_path.c_str(), openmode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fileDescriptor == -1)
	{
		TT_PANIC("Open file '%s' failed with error %d: %s", p_path.c_str(), errno, std::strerror(errno));
		
#if !defined(TT_BUILD_FINAL)
		struct stat entryStats = { 0 };
		if (stat(p_path.c_str(), &entryStats) == 0)
		{
			TT_Printf("PosixFileSystem::open: Entry '%s' permissions: %s%s%s%s%s%s%s%s%s%s (owned by %d:%d, this process is %d:%d)\n",
			          p_path.c_str(),
			          ((entryStats.st_mode & S_IFDIR) == S_IFDIR) ? "d" : "-",
			          ((entryStats.st_mode & S_IRUSR) == S_IRUSR) ? "r" : "-",
			          ((entryStats.st_mode & S_IWUSR) == S_IWUSR) ? "w" : "-",
			          ((entryStats.st_mode & S_IXUSR) == S_IXUSR) ? "x" : "-",
			          ((entryStats.st_mode & S_IRGRP) == S_IRGRP) ? "r" : "-",
			          ((entryStats.st_mode & S_IWGRP) == S_IWGRP) ? "w" : "-",
			          ((entryStats.st_mode & S_IXGRP) == S_IXGRP) ? "x" : "-",
			          ((entryStats.st_mode & S_IROTH) == S_IROTH) ? "r" : "-",
			          ((entryStats.st_mode & S_IWOTH) == S_IWOTH) ? "w" : "-",
			          ((entryStats.st_mode & S_IXOTH) == S_IXOTH) ? "x" : "-",
			          entryStats.st_uid, entryStats.st_gid,
			          getuid(), getgid());
		}
		
		std::string::size_type slash = p_path.rfind('/');
		if (slash != std::string::npos && stat(p_path.substr(0, slash).c_str(), &entryStats) == 0)
		{
			TT_Printf("PosixFileSystem::open: Entry '%s' permissions: %s%s%s%s%s%s%s%s%s%s (owned by %d:%d, this process is %d:%d)\n",
			          p_path.substr(0, slash).c_str(),
			          ((entryStats.st_mode & S_IFDIR) == S_IFDIR) ? "d" : "-",
			          ((entryStats.st_mode & S_IRUSR) == S_IRUSR) ? "r" : "-",
			          ((entryStats.st_mode & S_IWUSR) == S_IWUSR) ? "w" : "-",
			          ((entryStats.st_mode & S_IXUSR) == S_IXUSR) ? "x" : "-",
			          ((entryStats.st_mode & S_IRGRP) == S_IRGRP) ? "r" : "-",
			          ((entryStats.st_mode & S_IWGRP) == S_IWGRP) ? "w" : "-",
			          ((entryStats.st_mode & S_IXGRP) == S_IXGRP) ? "x" : "-",
			          ((entryStats.st_mode & S_IROTH) == S_IROTH) ? "r" : "-",
			          ((entryStats.st_mode & S_IWOTH) == S_IWOTH) ? "w" : "-",
			          ((entryStats.st_mode & S_IXOTH) == S_IXOTH) ? "x" : "-",
			          entryStats.st_uid, entryStats.st_gid,
			          getuid(), getgid());
		}
#endif
		return false;
	}
	
	PosixFsInternal* intern = new PosixFsInternal;
	intern->fileDescriptor = fileDescriptor;
	
	p_file->setData(intern);
	return true;
}


bool PosixFileSystem::close(File* p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		//TT_PANIC("File not open.");
		return false;
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("Attempt to close invalid file.");
		return false;
	}
	
	bool success = true;
	if (::close(intern->fileDescriptor) != 0)
	{
		TT_PANIC("Closing file failed with error %d: %s", errno, std::strerror(errno));
		success = false;
		// We'll need to clean up the internal data to prevent more leaks.
	}
	intern->fileDescriptor = -1;
	
	delete intern;
	p_file->setData(0);
	
	return success;
}


size_type PosixFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
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
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	size_t  numberOfBytesToRead = static_cast<size_t>(p_length);
	ssize_t numberOfBytesRead   = ::read(intern->fileDescriptor, p_buffer, numberOfBytesToRead);
	if (numberOfBytesRead == -1)
	{
		TT_PANIC("Reading from file failed with error %d: %s", errno, std::strerror(errno));
		return size_type();
	}
	
	return static_cast<size_type>(numberOfBytesRead);
}


size_type PosixFileSystem::readAsync(const FilePtr& p_file,
                                   void*          p_buffer,
                                   size_type      p_length,
                                   callback       p_callback,
                                   void*          p_arg)
{
	// FIXME: Implement this.
	(void)p_file;
	(void)p_buffer;
	(void)p_length;
	(void)p_callback;
	(void)p_arg;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return 0;
}


size_type PosixFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
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
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	size_t  numberOfBytesToWrite = static_cast<size_t>(p_length);
	ssize_t numberOfBytesWritten = ::write(intern->fileDescriptor, p_buffer, numberOfBytesToWrite);
	if (numberOfBytesWritten == -1)
	{
		TT_PANIC("Writing to file failed with error %d: %s", errno, std::strerror(errno));
		return size_type();
	}
	
	return static_cast<size_type>(numberOfBytesWritten);
}


size_type PosixFileSystem::writeAsync(const FilePtr& p_file,
                                    const void*    p_buffer,
                                    size_type      p_length,
                                    callback       p_callback,
                                    void*          p_arg)
{
	// FIXME: Implement this.
	(void)p_file;
	(void)p_buffer;
	(void)p_length;
	(void)p_callback;
	(void)p_arg;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return 0;
}



bool PosixFileSystem::flush(const FilePtr& /*p_file*/)
{
	// Flush isn't needed when using file descriptor I/O (write calls are flushed automatically)
	return true;
}


// ========== Asynchronous status functions ==========

bool PosixFileSystem::isBusy(const FilePtr& p_file)
{
	// FIXME: Implement this.
	(void)p_file;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return false;
}


bool PosixFileSystem::isSucceeded(const FilePtr& p_file)
{
	// FIXME: Implement this.
	(void)p_file;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return false;
}


bool PosixFileSystem::wait(const FilePtr& p_file)
{
	// FIXME: Implement this.
	(void)p_file;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return false;
}


bool PosixFileSystem::cancel(const FilePtr& p_file)
{
	// FIXME: Implement this.
	(void)p_file;
	TT_PANIC("This function is not supported yet on PosixFileSystem.");
	return false;
}


// ========== Position / size functions ==========

bool PosixFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	int origin;
	switch (p_position)
	{
	case SeekPos_Set: origin = SEEK_SET; break;
	case SeekPos_Cur: origin = SEEK_CUR; break;
	case SeekPos_End: origin = SEEK_END; break;
		
	default:
		TT_PANIC("Unknown seek mode: %d", p_position);
		return false;
	}
	
	if (::lseek(intern->fileDescriptor, static_cast<off_t>(p_offset), origin) == -1)
	{
		TT_PANIC("Seeking in file failed with error %d: %s", errno, std::strerror(errno));
		return false;
	}
	
	return true;
}


pos_type PosixFileSystem::getPosition(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return pos_type();
	}
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return pos_type();
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return pos_type();
	}
	
	off_t offset = ::lseek(intern->fileDescriptor, 0, SEEK_CUR);
	if (offset == -1)
	{
		TT_PANIC("Retrieving position in file failed with error %d: %s", errno, std::strerror(errno));
		return pos_type();
	}
	
	return static_cast<pos_type>(offset);
}


size_type PosixFileSystem::getLength(const FilePtr& p_file)
{
	struct stat entryInfo = { 0 };
	if (getFileStats(p_file, &entryInfo) == false)
	{
		return size_type();
	}
	
	return static_cast<size_type>(entryInfo.st_size);
}


// ========== Time functions ==========

time_type PosixFileSystem::getCreationTime(const FilePtr& p_file)
{
	struct stat entryInfo = { 0 };
	if (getFileStats(p_file, &entryInfo) == false)
	{
		return time_type();
	}
	
	return createTimeType(entryInfo.st_ctime);
}


time_type PosixFileSystem::getAccessTime(const FilePtr& p_file)
{
	struct stat entryInfo = { 0 };
	if (getFileStats(p_file, &entryInfo) == false)
	{
		return time_type();
	}
	
	return createTimeType(entryInfo.st_atime);
}


time_type PosixFileSystem::getWriteTime(const FilePtr& p_file)
{
	struct stat entryInfo = { 0 };
	if (getFileStats(p_file, &entryInfo) == false)
	{
		return time_type();
	}
	
	return createTimeType(entryInfo.st_mtime);
}

bool PosixFileSystem::setWriteTime(const FilePtr& p_file, time_type p_time)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	PosixFsInternal* intern = reinterpret_cast<PosixFsInternal*>(p_file->getData());
	if (intern->fileDescriptor == -1)
	{
		TT_PANIC("File not open.");
		return false;
	}

	struct timeval times[2];
	convertTimeType(times[0], p_time);
	convertTimeType(times[1], p_time);

	futimes(intern->fileDescriptor, times);
	return true;
}

s64 PosixFileSystem::convertToUnixTime(time_type p_nativeTime)
{
	return static_cast<s64>((p_nativeTime / SECONDS_TO_FILETIME) - EPOCH_DIFFERENCE_SECONDS);
}


time_type PosixFileSystem::convertToNativeTime(s64 p_unixTime)
{
	return static_cast<time_type>((p_unixTime + EPOCH_DIFFERENCE_SECONDS) * SECONDS_TO_FILETIME);
}


// ========== Directory functions ==========

bool PosixFileSystem::openDir(DirPtr& p_dir, const std::string& p_path, const std::string& p_filter)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_dir->getData() != 0)
	{
		TT_PANIC("Dir already open.");
		return false;
	}
	
	if (dirExists(p_path) == false)
	{
		TT_PANIC("Path '%s' not found.", p_path.c_str());
		return false;
	}
	
	// Attempt to open the specified directory
	DirInternal* intern = new DirInternal;
	
	intern->handle = opendir(p_path.c_str());
	if (intern->handle == 0)
	{
		TT_PANIC("Opening path '%s' failed with error %d: %s",
		         p_path.c_str(), errno, std::strerror(errno));
		delete intern;
		return false;
	}
	
	intern->ignoreHidden = true;
	intern->filter       = p_filter;
	intern->searchPath   = p_path;
	addTrailingDirSeparator(intern->searchPath);
	
	p_dir->setData(intern);
	
	return true;
}


bool PosixFileSystem::closeDir(Dir* p_dir)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_dir->getData() == 0)
	{
		//TT_PANIC("Dir not open.");
		return false;
	}
	
	DirInternal* intern = reinterpret_cast<DirInternal*>(p_dir->getData());
	if (intern->handle != 0)
	{
		if (closedir(intern->handle) != 0)
		{
			TT_PANIC("Closing dir failed with error %d: %s", errno, std::strerror(errno));
		}
		else
		{
			intern->handle = 0;
		}
	}
	
	delete intern;
	p_dir->setData(0);
	
	return true;
}


bool PosixFileSystem::readDir(const DirPtr& p_dir, DirEntry& p_entry)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_dir->getData() == 0)
	{
		TT_PANIC("Dir not open.");
		return false;
	}
	
	DirInternal* intern = reinterpret_cast<DirInternal*>(p_dir->getData());
	if (intern->handle == 0)
	{
		TT_PANIC("Dir not open.");
		return false;
	}
	
	// Read new directory entry
	struct dirent* entry = 0;
	for (bool keepSearching = true; keepSearching; )
	{
		entry = readdir(intern->handle);
		if (entry == 0)
		{
			// No more entries remain
			return false;
		}
		
		// Skip '.' and '..' entries
		if (strcmp(entry->d_name, ".")  == 0 ||
		    strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}
		
		// Skip hidden entries, if this was set (anything starting with '.' is hidden)
		if (intern->ignoreHidden && entry->d_name[0] == '.')
		{
			continue;
		}
		
		// Apply filter to entry (skip any entries that do not match the filter)
		if (utils::matchesFilter(entry->d_name, intern->filter) == false)
		{
			continue;
		}
		
		// Found an acceptabe entry; stop the search
		keepSearching = false;
	}
	
	// Get entry statistics
	const std::string fullPath(intern->searchPath + entry->d_name);
	struct stat entryStats = { 0 };
	if (stat(fullPath.c_str(), &entryStats) != 0)
	{
		TT_PANIC("Retrieving statistics for directory entry '%s' failed with error %d: %s",
		         fullPath.c_str(), errno, std::strerror(errno));
		return false;
	}
	
	/*
	TT_Printf("PosixFileSystem::readDir: Entry '%s' permissions: %s%s%s%s%s%s%s%s%s%s\n",
	          fullPath.c_str(),
	          ((entryStats.st_mode & S_IFDIR) == S_IFDIR) ? "d" : "-",
	          ((entryStats.st_mode & S_IRUSR) == S_IRUSR) ? "r" : "-",
	          ((entryStats.st_mode & S_IWUSR) == S_IWUSR) ? "w" : "-",
	          ((entryStats.st_mode & S_IXUSR) == S_IXUSR) ? "x" : "-",
	          ((entryStats.st_mode & S_IRGRP) == S_IRGRP) ? "r" : "-",
	          ((entryStats.st_mode & S_IWGRP) == S_IWGRP) ? "w" : "-",
	          ((entryStats.st_mode & S_IXGRP) == S_IXGRP) ? "x" : "-",
	          ((entryStats.st_mode & S_IROTH) == S_IROTH) ? "r" : "-",
	          ((entryStats.st_mode & S_IWOTH) == S_IWOTH) ? "w" : "-",
	          ((entryStats.st_mode & S_IXOTH) == S_IXOTH) ? "x" : "-");
	//*/
	
	// Set entry info
	p_entry.clear();
	
	p_entry.setName(entry->d_name);
	p_entry.setIsDirectory((entryStats.st_mode & S_IFDIR) == S_IFDIR);
	p_entry.setIsHidden(entry->d_name[0] == '.');
	if (p_entry.isDirectory() == false)
	{
		p_entry.setSize(static_cast<size_type>(entryStats.st_size));
	}
	p_entry.setCreationTime(static_cast<time_type>(entryStats.st_ctime));
	p_entry.setAccessTime(static_cast<time_type>(entryStats.st_atime));
	p_entry.setWriteTime(static_cast<time_type>(entryStats.st_mtime));
	
	permission_type perms = 0;
	perms |= ((entryStats.st_mode & S_IRUSR) == S_IRUSR) ? Permission_OwnerRead    : 0;
	perms |= ((entryStats.st_mode & S_IWUSR) == S_IWUSR) ? Permission_OwnerWrite   : 0;
	perms |= ((entryStats.st_mode & S_IXUSR) == S_IXUSR) ? Permission_OwnerExecute : 0;
	perms |= ((entryStats.st_mode & S_IRGRP) == S_IRGRP) ? Permission_GroupRead    : 0;
	perms |= ((entryStats.st_mode & S_IWGRP) == S_IWGRP) ? Permission_GroupWrite   : 0;
	perms |= ((entryStats.st_mode & S_IXGRP) == S_IXGRP) ? Permission_GroupExecute : 0;
	perms |= ((entryStats.st_mode & S_IROTH) == S_IROTH) ? Permission_OtherRead    : 0;
	perms |= ((entryStats.st_mode & S_IWOTH) == S_IWOTH) ? Permission_OtherWrite   : 0;
	perms |= ((entryStats.st_mode & S_IXOTH) == S_IXOTH) ? Permission_OtherExecute : 0;
	p_entry.setPermissions(perms);
	
	return true;
}


// ========== Create / Destroy functions ==========

bool PosixFileSystem::fileExists(const std::string& p_path)
{
	// Attempt to retrieve the status of the specified path
	struct stat entryInfo = { 0 };
	if (stat(p_path.c_str(), &entryInfo) != 0)
	{
		if (errno != ENOENT && errno != ENOTDIR)
		{
			TT_PANIC("Retrieving information about path '%s' failed with error %d: %s",
			         p_path.c_str(), errno, std::strerror(errno));
		}
		return false;
	}
	
	// An entry at the specified path exists; return whether it is a file
	return (entryInfo.st_mode & S_IFDIR) != S_IFDIR;
}


bool PosixFileSystem::destroyFile(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (unlink(p_path.c_str()) != 0)
	{
		TT_PANIC("Deleting file '%s' failed with error %d: %s",
		         p_path.c_str(), errno, std::strerror(errno));
		return false;
	}
	
	return true;
}


bool PosixFileSystem::copyFile(const std::string& p_existingFile,
                             const std::string& p_newFile,
                             bool               p_failIfExists)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No source path specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No destination path specified.");
		return false;
	}
	
	if (p_failIfExists && fileExists(p_newFile))
	{
		return false;
	}

	std::ifstream IN(p_existingFile.c_str(), std::ios::binary);
	std::ofstream OUT(p_newFile.c_str(), std::ios::binary);
	OUT << IN.rdbuf();
	OUT.flush();
	return true;
}


bool PosixFileSystem::moveFile(const std::string& p_existingFile,
                             const std::string& p_newFile,
                             bool               p_failIfExists)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No source path specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No destination path specified.");
		return false;
	}
	
	if (p_failIfExists && fileExists(p_newFile))
	{
		return false;
	}
	
	int result = rename(p_existingFile.c_str(), p_newFile.c_str());
	if (result != 0)
	{
		TT_PANIC("Moving file '%s' to '%s' failed with error %d: %s",
		         p_existingFile.c_str(), p_newFile.c_str(), result, std::strerror(result));
		return false;
	}
	
	return true;
}


bool PosixFileSystem::dirExists(const std::string& p_path)
{
	// Attempt to retrieve the status of the specified path
	struct stat entryInfo = { 0 };
	if (stat(p_path.c_str(), &entryInfo) != 0)
	{
		if (errno != ENOENT && errno != ENOTDIR)
		{
			TT_PANIC("Retrieving information about path '%s' failed with error %d: %s",
			         p_path.c_str(), errno, std::strerror(errno));
		}
		return false;
	}
	
	// An entry at the specified path exists; return whether it is a directory
	return (entryInfo.st_mode & S_IFDIR) == S_IFDIR;
}


bool PosixFileSystem::createDir(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (mkdir(p_path.c_str(), S_IRWXU | S_IRWXG) != 0)
	{
		TT_PANIC("Creating directory '%s' failed with error %d: %s",
		         p_path.c_str(), errno, std::strerror(errno));
		return false;
	}
	
	return true;
}


bool PosixFileSystem::destroyDir(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (rmdir(p_path.c_str()) != 0)
	{
		TT_PANIC("Removing directory '%s' failed with error %d: %s",
		         p_path.c_str(), errno, std::strerror(errno));
		return false;
	}
	
	return true;
}


// ========== Working Directory functions ==========

std::string PosixFileSystem::getWorkingDir()
{
	char buf[1024] = { 0 };
	if (getcwd(buf, 1024) == 0)
	{
		TT_PANIC("Retrieving current working directory failed with error %d: %s", errno, std::strerror(errno));
		return std::string();
	}
	
	std::string workingDir(buf);
	addTrailingDirSeparator(workingDir);
	return workingDir;
}


bool PosixFileSystem::setWorkingDir(const std::string& p_path)
{
	if (chdir(p_path.c_str()) == 0)
	{
		return true;
	}
	
	TT_PANIC("Setting new working directory to '%s' failed with error %d: %s",
	         p_path.c_str(), errno, std::strerror(errno));
	return false;
}

std::string PosixFileSystem::getApplicationDir()
{
#ifdef TT_PLATFORM_SDL
	char *base = SDL_GetBasePath();
	std::string appDir(base);
	SDL_free(base);

	return appDir;
#else
	// Get the full path to the executable
	std::string appPath = getApplicationPath();

	std::string::size_type pos = appPath.rfind('/');
	if (pos != std::string::npos)
	{
		return appPath.substr(0, pos + 1);
	}
	else
	{
		return std::string();
	}
#endif
}


std::string PosixFileSystem::getTemporaryDir()
{
	// Return whatever the environment says the temp directory is (ensure it has a trailing dir separator)
	const char *tmpdir = getenv("TMPDIR");
	if (tmpdir)
	{
		std::string tempDir(tmpdir);
		addTrailingDirSeparator(tempDir);
		return tempDir;
	}
	else
	{
		return "/tmp/";
	}
}

#ifdef TT_PLATFORM_LNX
std::string getLinuxSaveDir(const std::string& p_appName)
{
	const char *home = getenv("HOME");
	if (!home) return "";
	
	std::string pTemp(p_appName);
	for (std::string::iterator it = pTemp.begin(); it < pTemp.end(); ++it) {
		*it = tolower(*it);
		if (std::isspace(*it) || *it == '/') {
			*it = '_';
		}
	}

	std::string rootDir(home);
	if (*rootDir.rbegin() != '/') rootDir += "/";

	rootDir += ".twotribes/" + pTemp + '/';

	return rootDir;
}

std::string getSDL2LowerCaseSaveDir(const std::string& p_appName)
{
	std::string ret;

	std::string pTemp(p_appName);
	for (std::string::iterator it = pTemp.begin(); it < pTemp.end(); ++it) {
		*it = tolower(*it);
		if (std::isspace(*it) || *it == '/') {
			*it = '_';
		}
	}
	char *base = SDL_GetPrefPath("twotribes", pTemp.c_str());
	ret = base;
	SDL_free(base);

	return ret;
}
#endif

std::string PosixFileSystem::getSaveRootDir()
{
#ifdef TT_PLATFORM_SDL
	if (m_saveDir.empty()) {
#ifdef TT_PLATFORM_LNX
		// Try OLD linux save dir
		std::string oldLinuxSaveDir = getLinuxSaveDir(m_appName);
		if (!oldLinuxSaveDir.empty() && dirExists(oldLinuxSaveDir)) {
			m_saveDir = oldLinuxSaveDir;
		}

		if (m_saveDir.empty())
		{
			// try TT2 linux save dir
			std::string sdlLowerSaveDir = getSDL2LowerCaseSaveDir(m_appName);
			if (!sdlLowerSaveDir.empty() && dirExists(sdlLowerSaveDir)) {
				m_saveDir = sdlLowerSaveDir;
			}
		}
#endif
#ifdef TT_PLATFORM_OSX_MAC
		std::string oldMacSaveDir = tt::system::getOldMacOSXSavePath();
		if (!oldMacSaveDir.empty() && dirExists(oldMacSaveDir)) {
			m_saveDir = oldMacSaveDir;
		}
#endif

		if (m_saveDir.empty())
		{
			// New savedir
			char *base = SDL_GetPrefPath("Two Tribes", m_appName.c_str());
			m_saveDir = base;
			SDL_free(base);
		}
	}
	return m_saveDir;
#else
	const char *home = getenv("HOME");
	
	std::string pTemp(m_appName);
	for (std::string::iterator it = pTemp.begin(); it < pTemp.end(); ++it) {
		*it = tolower(*it);
		if (std::isspace(*it) || *it == '/') {
			*it = '_';
		}
	}

	std::string rootDir(home);
	addTrailingDirSeparator(rootDir);
	rootDir += ".twotribes/" + pTemp + '/';

	return rootDir;
#endif
}


// ========== Path functions ==========

std::string PosixFileSystem::getAbsolutePath(const std::string& p_path)
{
	char outPath[1024];
	if (realpath(p_path.c_str(), outPath) == 0)
	{
		TT_PANIC("Resolving '%s' to an absolute path failed with error %d: %s",
		         p_path.c_str(), errno, std::strerror(errno));
		return std::string();
	}
	return outPath;
}


std::string PosixFileSystem::getRelativePath(const std::string& p_path)
{
	return getRelativePathTo(p_path, getWorkingDir());
}


std::string PosixFileSystem::getRelativePathTo(const std::string& p_path, const std::string& p_dir)
{
	std::string cwd(getAbsolutePath(p_dir));
	if (cwd.empty())
	{
		TT_PANIC("Unable to get directory.");
		return std::string();
	}
	
	std::string::value_type separator = getDirSeparator();
	if (cwd.at(cwd.length() - 1) != separator)
	{
		cwd += separator;
	}
	
	std::string abs = getAbsolutePath(p_path);
	if (abs.empty())
	{
		TT_PANIC("Unable to get absolute path.");
		return std::string();
	}
	
	// Check whether cwd or abs is a network dir/file
	bool cwdnetwork = cwd.length() >= 2 && cwd.at(0) == separator && cwd.at(1) == separator;
	bool absnetwork = abs.length() >= 2 && abs.at(0) == separator && abs.at(1) == separator;
	if (cwdnetwork != absnetwork)
	{
		// One of the two paths is a network path
		return std::string();
	}
	
	if (cwdnetwork)
	{
		// Network path, server and share must match
		std::string::size_type cwdserverpos = cwd.find(separator, 2);
		if (cwdserverpos == std::string::npos)
		{
			TT_PANIC("Current working directory '%s' is an invalid network name.", cwd.c_str());
			return std::string();
		}
		std::string cwdserver = cwd.substr(2, cwdserverpos - 2);
		
		std::string::size_type cwdsharepos = cwd.find(separator, cwdserverpos + 1);
		if (cwdsharepos == std::string::npos)
		{
			TT_PANIC("Current working directory '%s' is an invalid network name.", cwd.c_str());
			return std::string();
		}
		std::string cwdshare = cwd.substr(cwdserverpos + 1, cwdsharepos - (cwdserverpos + 1));
		
		std::string::size_type absserverpos = abs.find(separator, 2);
		if (absserverpos == std::string::npos)
		{
			TT_PANIC("Path '%s' is an invalid network name.", abs.c_str());
			return std::string();
		}
		std::string absserver = abs.substr(2, absserverpos - 2);
		
		std::string::size_type abssharepos = abs.find(separator, absserverpos + 1);
		if (abssharepos == std::string::npos)
		{
			TT_PANIC("Path '%s' is an invalid network name.", abs.c_str());
			return std::string();
		}
		std::string absshare = abs.substr(absserverpos + 1, abssharepos - (absserverpos + 1));
		
		if (cwdserver != absserver)
		{
			// Server mismatch
			return std::string();
		}
		
		if (cwdshare != absshare)
		{
			// Share mismatch
			return std::string();
		}
	}
	
	// Get common part of both strings
	typedef std::pair<std::string::iterator, std::string::iterator> itpair;
	if (cwd.length() > abs.length())
	{
		itpair common = std::mismatch(abs.begin(), abs.end(), cwd.begin());
		
		// Search back until first backslash
		while (common.first != abs.begin())
		{
			if (*(common.first - 1) == separator)
			{
				break;
			}
			--common.first;
			--common.second;
		}
		if (common.first == abs.begin())
		{
			// Full mismatch
			return std::string();
		}
		
		abs = std::string(common.first, abs.end());
		cwd = std::string(common.second, cwd.end());
	}
	else
	{
		itpair common = std::mismatch(cwd.begin(), cwd.end(), abs.begin());
		
		// Search back until first directory separator
		while (common.first != cwd.begin())
		{
			if (*(common.first - 1) == separator)
			{
				break;
			}
			--common.first;
			--common.second;
		}
		if (common.first == cwd.begin())
		{
			// Full mismatch
			return std::string();
		}
		
		cwd = std::string(common.first, cwd.end());
		abs = std::string(common.second, abs.end());
	}
	
	while (cwd.empty() == false)
	{
		// For every directory remaining in cwd, add "..<separator>" to abs
		std::string::size_type pos = cwd.find(separator);
		
		// Current working dir always ends with directory separator
		abs = std::string("..") + separator + abs;
		cwd = cwd.substr(pos + 1);
	}
	
	return abs;
}


std::string::value_type PosixFileSystem::getDirSeparator() const
{
	return std::string::value_type('/');
}


std::string PosixFileSystem::getApplicationPath()
{
	// Get the full path to the executable
	std::string appPath;
#ifdef TT_PLATFORM_SDL
	TT_PANIC("Not implemented!");
#else
	char buffer[1024];
	ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
	
	if (length != -1)
	{
		buffer[length] = '\0';
		appPath.assign(buffer);
	}
	else
	{
		TT_PANIC("Retrieving executable path failed");
	}

#endif
	return appPath;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

PosixFileSystem::PosixFileSystem(fs::identifier p_id, const std::string& p_appName)
:
FileSystem(p_id),
m_appName(p_appName)
{
}


void PosixFileSystem::addTrailingDirSeparator(std::string& p_path)
{
	if (p_path.empty() == false && *p_path.rbegin() != getDirSeparator())
	{
		p_path += getDirSeparator();
	}
}

// Namespace end
}
}
