#include <algorithm>

#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#if defined(_XBOX)
#include <xtl.h>
#else
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#endif

#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/WindowsFileSystem.h>
#include <tt/mem/util.h>
#include <tt/str/str.h>


// Set this to 1 to enable checking of filename case when opening a file for reading
#if defined(TT_BUILD_FINAL) || defined(_XBOX)
#define TT_WINFS_VALIDATE_FILENAME_CASING 0 // Do NOT change this value! (Final Build)
#else
#define TT_WINFS_VALIDATE_FILENAME_CASING 1
#endif


namespace tt {
namespace fs {

struct WinFsInternal
{
	HANDLE     handle;
	OVERLAPPED overlapped;
	HANDLE     event;
	HANDLE     thread;
	callback   callbackfunction;
	void*      arg;
	DWORD      exitcode;
	
	WinFsInternal()
	:
	handle(INVALID_HANDLE_VALUE),
	event(CreateEvent(NULL, FALSE, FALSE, NULL)),
	thread(INVALID_HANDLE_VALUE),
	callbackfunction(0),
	arg(0),
	exitcode(0)
	{
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = event;
	}
	
	~WinFsInternal()
	{
		if ( handle != INVALID_HANDLE_VALUE )
		{
			// warn
			if ( CloseHandle(handle) != TRUE )
			{
				// warn
			}
		}
		
		// event is handled by WinFsInternal
		if ( CloseHandle(event) != TRUE )
		{
			// warn
		}
		
		if ( thread != INVALID_HANDLE_VALUE )
		{
			// warn
			if ( CloseHandle(thread) != TRUE )
			{
				// warn
			}
		}
	}
};


struct DirInternal
{
	HANDLE           handle;
	WIN32_FIND_DATAA finddata;
	bool             ignoreHidden;
};


inline time_type createTimeType(const FILETIME& p_time)
{
	time_type time = static_cast<time_type>(p_time.dwHighDateTime) << 32;
	time |= static_cast<time_type>(p_time.dwLowDateTime);
	return time;
}


inline FILETIME createFileTime(time_type p_time)
{
	FILETIME result;
	result.dwHighDateTime = p_time >> 32;
	result.dwLowDateTime  = static_cast<DWORD>(p_time);	// truncate upper bits
	return result;
}


#if !defined(TT_BUILD_FINAL) && !defined(_XBOX)

#if 0  // Original implementation disabled, because of false positives on Windows 7
/*! \brief Helper function to determine the real on-disk casing of a path. Requires native path as input.
    \param p_path The path for which to get the real casing (must be a native path, using backslashes).
    \return Casing-correct version of the input path.
    \note Copied and modified from http://efreedom.com/Question/1-74451/Getting-Actual-File-Name-Proper-Casing-Windows */
inline std::string getActualPathName(const std::string& p_path)
{
	const std::string::value_type separator = '\\';
	
	// Copy input string because we'll be temporarily modifying it in place
	const std::string::size_type length = p_path.length();
	std::string::value_type*     buffer = new std::string::value_type[length + 1];
	mem::copy8(buffer, p_path.c_str(), (length + 1) * sizeof(std::string::value_type));
	
	std::string::size_type i = 0;
	
	std::string result;
	result.reserve(length);
	
	// For network paths (\\server\share\RestOfPath), getting the display name mangles it into
	// unusable form (e.g. "\\server\share" turns into "share on server (server)").
	// So detect this case and just skip up to two path components
	if (length >= 2 && buffer[0] == separator && buffer[1] == separator)
	{
		s32 skippedCount = 0;
		i = 2; // Start after '\\'
		while (i < length && skippedCount < 2)
		{
			if (buffer[i] == separator)
			{
				++skippedCount;
			}
			++i;
		}
		
		result.append(buffer, i);
	}
	// For drive names, leave casing intact
	else if (length >= 2 && buffer[1] == ':')
	{
		result += buffer[0];
		result += ':';
		if (length >= 3 && buffer[2] == separator)
		{
			result += separator;
			i = 3; // Start after drive, colon and separator
		}
		else
		{
			i = 2; // Start after drive and colon
		}
	}
	
	size_t lastComponentStart = i;
	bool   addSeparator       = false;
	
	while (i < length)
	{
		// Skip until path separator
		while (i < length && buffer[i] != separator)
		{
			++i;
		}
		
		if (addSeparator)
		{
			result += separator;
		}
		
		// If we found path separator, get real filename of this last path name component
		const bool foundSeparator = (i < length);
		buffer[i] = 0;
		
		// Nuke the path separator so that we get real name of current path component
		const size_t originalComponentLength = (i - lastComponentStart);
		SHFILEINFOA info = { 0 };
		if (SHGetFileInfoA(buffer, 0, &info, sizeof(info), SHGFI_DISPLAYNAME) != 0)
		{
			if (std::strlen(info.szDisplayName) == originalComponentLength)
			{
				result += info.szDisplayName;
			}
			else
			{
				// Path component resolved to a string of different length:
				// assume this is a special Windows directory which gets displayed
				// differently from what is actually on disk
				// (and as such, use original path name component)
				result.append(buffer + lastComponentStart, originalComponentLength);
			}
		}
		else
		{
			// Most likely file does not exist, so just append original path name component
			result.append(buffer + lastComponentStart, originalComponentLength);
		}
		
		// Restore path separator that we might have nuked before
		if (foundSeparator)
		{
			buffer[i] = separator;
		}
		
		++i;
		lastComponentStart = i;
		addSeparator       = true;
	}
	
	delete[] buffer;
	buffer = 0;
	
	return result;
}

#else  // Alternative implementation

/*! \brief Helper function to determine the real on-disk casing of a path. Requires native path as input.
    \param p_path The path for which to get the real casing (must be a native path, using backslashes).
    \return Casing-correct version of the input path. */
inline std::string getActualPathName(const std::string& p_path)
{
	// One way to get the actual on-disk casing of a path is to convert the path to a "short path" first,
	// then convert that short path back into a "long path". However, this does not always work.
	// (for example, it fails if the application does not have rights to list directory contents
	//  of one of the path components)
	
	// First turn the input path into a short path
	std::unique_ptr<char[]> shortPathBuffer;
	{
		DWORD shortPathLength = 0;
		
		// Get the buffer size needed
		shortPathLength = GetShortPathNameA(p_path.c_str(), 0, 0);
		if (shortPathLength == 0)
		{
			TT_PANIC("Retrieving short path length for path '%s' failed.", p_path.c_str());
			return p_path;
		}
		
		// Create required buffer and convert the path for real
		shortPathBuffer.reset(new char[shortPathLength]);
		shortPathLength = GetShortPathNameA(p_path.c_str(), shortPathBuffer.get(), shortPathLength);
		if (shortPathLength == 0)
		{
			TT_PANIC("Converting path '%s' to a short path failed.", p_path.c_str());
			return p_path;
		}
	}
	
	// Now convert the short path back into a long path
	DWORD longPathLength = GetLongPathNameA(shortPathBuffer.get(), 0, 0);
	if (longPathLength == 0)
	{
		TT_PANIC("Retrieving long path length for short path '%s' failed.\nOriginal path: '%s'",
		         shortPathBuffer.get(), p_path.c_str());
		return p_path;
	}
	
	std::unique_ptr<char[]> longPathBuffer(new char[longPathLength]);
	longPathLength = GetLongPathNameA(shortPathBuffer.get(), longPathBuffer.get(), longPathLength);
	if (longPathLength == 0)
	{
		TT_PANIC("Converting short path '%s' to a long path failed.\nOriginal path: '%s'",
		         shortPathBuffer.get(), p_path.c_str());
		return p_path;
	}
	
	return longPathBuffer.get();
}

#endif


#endif  // !defined(TT_BUILD_FINAL) && !defined(_XBOX)



//--------------------------------------------------------------------------------------------------
// Public member functions

FileSystemPtr WindowsFileSystem::instantiate(fs::identifier p_identifier,
                                             const std::string& p_appName)
{
	FileSystemPtr filesys(new WindowsFileSystem(p_identifier, p_appName));
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


WindowsFileSystem::~WindowsFileSystem()
{
}


// Feature support check functions

bool WindowsFileSystem::supportsSaving()
{
	return true;
}


bool WindowsFileSystem::supportsDirectories()
{
	return true;
}


bool WindowsFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
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
	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode     = 0;
	DWORD dwCreationDisposition = 0;
	
	switch (p_mode & (~OpenMode_AtEnd))
	{
	case OpenMode_Read:
		dwDesiredAccess = GENERIC_READ;
		dwShareMode = FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
		break;
		
	case OpenMode_Write:
	case OpenMode_Write | OpenMode_Truncate:
		dwDesiredAccess = GENERIC_WRITE;
		dwShareMode = 0;
		dwCreationDisposition = CREATE_ALWAYS;
		break;
		
	case OpenMode_Append:
		p_mode = static_cast<OpenMode>(static_cast<int>(p_mode) | OpenMode_AtEnd);
		// FALL THROUGH
	case OpenMode_Write | OpenMode_Append:
		dwDesiredAccess = GENERIC_WRITE;
		dwShareMode = 0;
		dwCreationDisposition = OPEN_ALWAYS;
		break;
		
	case OpenMode_Read | OpenMode_Write:
	case OpenMode_Read | OpenMode_Write | OpenMode_Append:
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = 0;
		dwCreationDisposition = OPEN_EXISTING;
		break;
		
	case OpenMode_Read | OpenMode_Write | OpenMode_Truncate:
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = 0;
		dwCreationDisposition = TRUNCATE_EXISTING;
		break;
		
	default:
		TT_PANIC("Unsupported open mode (0x%08X).", p_mode & (~OpenMode_AtEnd));
		return false;
	}
	
	HANDLE handle = CreateFileA(p_path.c_str(),
	                            dwDesiredAccess,
	                            dwShareMode,
	                            0,
	                            dwCreationDisposition,
	                            0,
	                            0);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		char buffer[1024] = { 0 };
#if !defined(_XBOX)
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 1033, buffer, 1024, 0);
#endif
		TT_PANIC("Unable to open file '%s':\n%s", p_path.c_str(), buffer);
		return false;
	}
	
#if TT_WINFS_VALIDATE_FILENAME_CASING
	// Check to make sure that the filename casing for read mode is exactly as it is on-disk
	if (dwCreationDisposition == OPEN_EXISTING || dwCreationDisposition == TRUNCATE_EXISTING)
	{
		validatePathCasing(p_path, getIdentifier());
	}
#endif
	
	WinFsInternal* intern = new WinFsInternal;
	if (intern->event == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("Unable to create internal file data.");
		delete intern;
		return false;
	}
	intern->handle = handle;
	
	p_file->setData(reinterpret_cast<void*>(intern));
	
	if ((p_mode & OpenMode_AtEnd) != 0)
	{
		if (SetFilePointer(intern->handle, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)
		{
			TT_PANIC("Error while seeking.");
			CloseHandle(intern->handle);
			
			delete intern;
			return false;
		}
	}
	return true;
}


bool WindowsFileSystem::close(File* p_file)
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
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (intern->handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("Attempt to close invalid file handle.");
		return false;
	}
	
	bool success = true;
	if (CloseHandle(intern->handle) != TRUE)
	{
		TT_PANIC("Failed to close file.");
		success = false;
		// we'll need to clean up the internal data to prevent more leaks.
	}
	else
	{
		intern->handle = INVALID_HANDLE_VALUE;
	}
	
	delete intern;
	p_file->setData(0);
	
	return success;
}


size_type WindowsFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
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
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	DWORD nNumberOfBytesToRead = static_cast<DWORD>(p_length);
	DWORD nNumberOfBytesRead = 0;
	if (ReadFile(handle, p_buffer, nNumberOfBytesToRead, &nNumberOfBytesRead, 0) == 0)
	{
		TT_PANIC("Error reading from file %s. %d", p_file->getPath(), GetLastError());
		return size_type();
	}
	
	size_type ret = static_cast<size_type>(nNumberOfBytesRead);
	return ret;
}


size_type WindowsFileSystem::readAsync(const FilePtr& p_file,
                                       void*          p_buffer,
                                       size_type      p_length,
                                       callback       p_callback,
                                       void*          p_arg)
{
	size_type ret = read(p_file, p_buffer, p_length);
	if (p_callback != 0)
	{
		p_callback(p_file, p_arg);
	}
	return ret;
}


size_type WindowsFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
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
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	DWORD nNumberOfBytesToWrite = static_cast<DWORD>(p_length);
	DWORD nNumberOfBytesWritten = 0;
	if (WriteFile(handle, p_buffer, nNumberOfBytesToWrite, &nNumberOfBytesWritten, 0) == 0)
	{
		char buffer[1024] = { 0 };
#if !defined(_XBOX)
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 1033, buffer, 1024, 0);
#endif
		TT_PANIC("Error writing %d bytes to file %s:\n%s", p_length, p_file->getPath(), buffer);
		
		return size_type();
	}
	
	size_type ret = static_cast<size_type>(nNumberOfBytesWritten);
	return ret;
}


size_type WindowsFileSystem::writeAsync(const FilePtr& p_file,
                                        const void*    p_buffer,
                                        size_type      p_length,
                                        callback       p_callback,
                                        void*          p_arg)
{
	size_type ret = write(p_file, p_buffer, p_length);
	if (p_callback != 0)
	{
		p_callback(p_file, p_arg);
	}
	return ret;
}


bool WindowsFileSystem::flush(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (FlushFileBuffers(intern->handle) == 0)
	{
		TT_PANIC("Flushing buffers failed.");
		return false;
	}
	return true;
}


bool WindowsFileSystem::isBusy(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (intern->thread == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}


bool WindowsFileSystem::isSucceeded(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (intern->exitcode != 0)
	{
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::wait(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (intern->thread == INVALID_HANDLE_VALUE)
	{
		return true;
	}
	
	DWORD ret = WaitForSingleObject(intern->event, INFINITE);
	if (ret == WAIT_FAILED)
	{
		TT_PANIC("Waiting for file failed.");
		return false;
	}
	
	if (ret != WAIT_OBJECT_0)
	{
		TT_PANIC("Waiting for file failed.");
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::cancel(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	if (intern->thread == INVALID_HANDLE_VALUE)
	{
		return true;
	}
	
	if (CancelIo(intern->handle) != TRUE)
	{
		TT_PANIC("Unable to cancel.");
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	LONG lDistanceToMove = static_cast<LONG>(p_offset);
	DWORD dwMoveMethod;
	switch (p_position)
	{
	case SeekPos_Set: dwMoveMethod = FILE_BEGIN;   break;
	case SeekPos_Cur: dwMoveMethod = FILE_CURRENT; break;
	case SeekPos_End: dwMoveMethod = FILE_END;     break;
	default:
		TT_PANIC("Unsupported seek mode: %d", p_position);
		return false;
	}
	
	if (SetFilePointer(intern->handle, lDistanceToMove, 0, dwMoveMethod) == INVALID_SET_FILE_POINTER)
	{
		TT_PANIC("Error while seeking.");
		return false;
	}
	
	return true;
}


pos_type WindowsFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	
	// since the win api lacks a GetFilePointer function,
	// we will seek 0 bytes from our current position,
	// this will return the current position
	DWORD filepointer = SetFilePointer(intern->handle, 0, 0, FILE_CURRENT);
	if (filepointer == INVALID_SET_FILE_POINTER)
	{
		TT_PANIC("Error getting file pointer.");
		return pos_type();
	}
	
	return static_cast<pos_type>(filepointer);
}


size_type WindowsFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return size_type();
	}
	
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle(handle, &info) == 0)
	{
		TT_PANIC("Unable to get file information.");
		return size_type();
	}
	
	if (info.nFileSizeHigh != 0)
	{
		TT_WARN("File over 4 GB in size, size will be truncated.");
	}
	
	return static_cast<size_type>(info.nFileSizeLow);
}


time_type WindowsFileSystem::getCreationTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return time_type();
	}
	
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle(handle, &info) == 0)
	{
		TT_PANIC("Unable to get file information.");
		return time_type();
	}
	
	return createTimeType(info.ftCreationTime);
}


time_type WindowsFileSystem::getAccessTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return time_type();
	}
	
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle(handle, &info) == 0)
	{
		TT_PANIC("Unable to get file information.");
		return time_type();
	}
	
	return createTimeType(info.ftLastAccessTime);
}


time_type WindowsFileSystem::getWriteTime(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return time_type();
	}
	
	BY_HANDLE_FILE_INFORMATION info;
	if (GetFileInformationByHandle(handle, &info) == 0)
	{
		TT_PANIC("Unable to get file information.");
		return time_type();
	}
	
	return createTimeType(info.ftLastWriteTime);
}


bool WindowsFileSystem::setWriteTime(const FilePtr& p_file, time_type p_time)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	WinFsInternal* intern = reinterpret_cast<WinFsInternal*>(p_file->getData());
	HANDLE handle = intern->handle;
	
	if (handle == 0 || handle == INVALID_HANDLE_VALUE)
	{
		TT_PANIC("File not open.");
		return false;
	}
	
	const FILETIME time = createFileTime(p_time);
	
	if (SetFileTime(handle, 0, 0, &time) == 0)
	{
		const DWORD errCode      = GetLastError();
		char        message[1024] = { 0 };
#if !defined(_XBOX)
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 1033, message, 1024, 0);
#endif
		TT_PANIC("Unable to set file write time.\nError (code 0x%08X): %s",
		         errCode, message);
		return false;
	}
	
	return true;
}


s64 WindowsFileSystem::convertToUnixTime(time_type p_nativeTime)
{
	// UNIX time is the number of seconds since January 1, 1970.
	// time_type is a 64-bit value for the number of 100-nanosecond periods since January 1, 1601.
	// Convert by subtracting the number of 100-nanosecond period between 1970-01-01 and 1601-01-01,
	// then divide by 1e+7 to get to the same base granularity.
	return static_cast<s64>((p_nativeTime - 116444736000000000ui64) / 10000000ui64);
}


time_type WindowsFileSystem::convertToNativeTime(s64 p_unixTime)
{
	// UNIX time is the number of seconds since January 1, 1970.
	// time_type is a 64-bit value for the number of 100-nanosecond periods since January 1, 1601.
	// Convert by multiplying the value by 1e+7 to get to the same base granularity,
	// then add the numeric equivalent of January 1, 1970 as time_type.
	return static_cast<time_type>((p_unixTime * 10000000ui64) + 116444736000000000ui64);
}


bool WindowsFileSystem::openDir(DirPtr& p_dir, const std::string& p_path, const std::string& p_filter)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return 0;
	}
	
	if (p_dir->getData() != 0)
	{
		TT_PANIC("Dir already open.");
		return false;
	}
	
	if (dirExists(p_path) == false)
	{
		TT_PANIC("Directory path '%s' not found.", p_path.c_str());
		return false;
	}
	
#if TT_WINFS_VALIDATE_FILENAME_CASING
	validatePathCasing(p_path, getIdentifier());
#endif
	
	DirInternal* intern = new DirInternal;
	
	intern->handle = FindFirstFileA((p_path + "\\" + p_filter).c_str(), & intern->finddata);
	intern->ignoreHidden = true;
	
	p_dir->setData(reinterpret_cast<void*>(intern));
	
	return true;
}


bool WindowsFileSystem::closeDir(Dir* p_dir)
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
	if (intern->handle != INVALID_HANDLE_VALUE)
	{
		if (FindClose(intern->handle) == 0)
		{
			TT_PANIC("Error closing dir.");
		}
		else
		{
			intern->handle = INVALID_HANDLE_VALUE;
		}
	}
	
	delete intern;
	p_dir->setData(0);
	
	return true;
}


bool WindowsFileSystem::readDir(const DirPtr& p_dir, DirEntry& p_entry)
{
	if (validate(p_dir) == false)
	{
		return false;
	}
	
	DirInternal* intern = reinterpret_cast<DirInternal*>(p_dir->getData());
	if (intern->handle == INVALID_HANDLE_VALUE)
	{
		// empty folder
		return false;
	}
	
	p_entry.setName(intern->finddata.cFileName);
	p_entry.setIsDirectory(
		(intern->finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	p_entry.setIsHidden(
		(intern->finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN);
	
	// set times
	p_entry.setCreationTime(createTimeType(intern->finddata.ftCreationTime));
	p_entry.setAccessTime(createTimeType(intern->finddata.ftLastAccessTime));
	p_entry.setWriteTime(createTimeType(intern->finddata.ftLastWriteTime));
	
	// set size
	if (intern->finddata.nFileSizeHigh != 0)
	{
		TT_WARN("File over 4 GB, size will be truncated.");
	}
	p_entry.setSize(static_cast<size_type>(intern->finddata.nFileSizeLow));
	
	bool done = false;
	do
	{
		if (FindNextFileA(intern->handle, &intern->finddata) == 0)
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				if (FindClose(intern->handle) == 0)
				{
					TT_PANIC("Unable to close dir.");
				}
				intern->handle = INVALID_HANDLE_VALUE;
			}
			else
			{
				TT_PANIC("Error reading from dir.");
			}
			done = true;
		}
		else if(intern->ignoreHidden &&
		        (intern->finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)
		{
			done = true;
		}
	}
	while (done == false);
	
	return true;
}


bool WindowsFileSystem::fileExists(const std::string& p_path)
{
	if (p_path.empty())
	{
		return false;
	}
	
	DWORD attributes = GetFileAttributesA(p_path.c_str());
#if defined(_XBOX)
	static const DWORD errorReturnValue = static_cast<DWORD>(-1);
#else
	static const DWORD errorReturnValue = INVALID_FILE_ATTRIBUTES;
#endif
	if (attributes == errorReturnValue)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND &&
		    error != ERROR_PATH_NOT_FOUND)
		{
			TT_PANIC("Error getting file attributes for '%s' (error: %d).",
			         p_path.c_str(), error);
		}
		return false;
	}
	
	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::destroyFile(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (DeleteFileA(p_path.c_str()) == 0)
	{
		TT_PANIC("Error deleting %s.", p_path.c_str());
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::copyFile(const std::string& p_existingFile,
                                 const std::string& p_newFile,
                                 bool               p_failIfExists)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (CopyFileA(p_existingFile.c_str(), p_newFile.c_str(), p_failIfExists ? TRUE : FALSE) == 0)
	{
		const DWORD errCode = GetLastError();
		
		// Target might be read-only; check if target exists, disable offending attributes, try again
		// (only if we were told not to fail on existing files)
		if (p_failIfExists == false && fileExists(p_newFile))
		{
			if (SetFileAttributesA(p_newFile.c_str(), FILE_ATTRIBUTE_NORMAL))
			{
				if (CopyFileA(p_existingFile.c_str(), p_newFile.c_str(), p_failIfExists ? TRUE : FALSE))
				{
					return true;
				}
			}
		}
		
		if (p_failIfExists == false || errCode != ERROR_FILE_EXISTS)
		{
			char msg[256] = { 0 };
#if !defined(_XBOX)
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 0, msg, 256, 0);
#endif
			TT_PANIC("Error copying '%s' to '%s'. Code 0x%08X: '%s'",
			         p_existingFile.c_str(), p_newFile.c_str(), errCode, msg);
		}
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::moveFile(const std::string& p_existingFile,
                                 const std::string& p_newFile,
                                 bool               p_failIfExists)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	DWORD flags = MOVEFILE_WRITE_THROUGH; // does not return until file is actually moved
	if (p_failIfExists == false)
	{
		flags |= MOVEFILE_REPLACE_EXISTING;
	}
	
	if (MoveFileExA(p_existingFile.c_str(), p_newFile.c_str(), flags) == 0)
	{
		TT_PANIC("Error moving %s to %s.", p_existingFile.c_str(), p_newFile.c_str());
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::dirExists(const std::string& p_path)
{
	if (p_path.empty())
	{
		return false;
	}
	
	DWORD attributes = GetFileAttributesA(p_path.c_str());
#if defined(_XBOX)
	static const DWORD errorReturnValue = static_cast<DWORD>(-1);
#else
	static const DWORD errorReturnValue = INVALID_FILE_ATTRIBUTES;
#endif
	if (attributes == errorReturnValue)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND &&
		    error != ERROR_PATH_NOT_FOUND)
		{
			TT_PANIC("Error getting file attributes for directory '%s'", p_path.c_str());
		}
		return false;
	}
	
	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
	{
		return false;
	}
	
	return true;
}


bool WindowsFileSystem::createDir(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (CreateDirectoryA(p_path.c_str(), 0) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		TT_PANIC("Error creating directory '%s' (%d).", p_path.c_str(), GetLastError());
		return false;
	}
	return true;
}


bool WindowsFileSystem::destroyDir(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (RemoveDirectoryA(p_path.c_str()) == 0)
	{
		TT_PANIC("Error deleting directory %s.", p_path.c_str());
		return false;
	}
	
	return true;
}


// Working Directory functions

std::string WindowsFileSystem::getWorkingDir()
{
#if defined(_XBOX)
	TT_PANIC("Working directory is not supported on Xbox 360.");
	return std::string();
#else
	char cwd[MAX_PATH] = { 0 };
	if (GetCurrentDirectoryA(MAX_PATH, cwd) == 0)
	{
		TT_PANIC("Error retrieving current working directory (err: %d).", GetLastError());
		return std::string();
	}
	return std::string(cwd) + "\\";
#endif
}


bool WindowsFileSystem::setWorkingDir(const std::string& p_path)
{
#if defined(_XBOX)
	(void)p_path;
	TT_PANIC("Working directory is not supported on Xbox 360.");
	return false;
#else
	if (SetCurrentDirectoryA(p_path.c_str()) == 0)
	{
		TT_PANIC("Unable to set current working directory to '%s' (err: %d).",
		         p_path.c_str(), GetLastError());
		return false;
	}
	return true;
#endif
}


std::string WindowsFileSystem::getApplicationDir()
{
#if defined(_XBOX)
	TT_PANIC("Not implemented on Xbox 360.");
	return std::string();
#else
	char appname[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(NULL, appname, MAX_PATH) == 0)
	{
		DWORD errCode     = GetLastError();
		char  errMsg[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 0, errMsg, 256, 0);
		TT_PANIC("Error retrieving application path (error %d: '%s').", errCode, errMsg);
		return std::string();
	}
	
	std::string app(appname);
	std::string::size_type pos = app.rfind('\\');
	if (pos != std::string::npos)
	{
		return app.substr(0, pos + 1);
	}
	
	return std::string();
#endif
}


std::string WindowsFileSystem::getTemporaryDir()
{
#if defined(_XBOX)
	// FIXME: Return real temp dir for Xbox 360
	TT_PANIC("Not implemented on Xbox 360 yet.");
	return std::string();
#else
	char tmp[MAX_PATH] = { 0 };
	if (GetTempPathA(MAX_PATH, tmp) == 0)
	{
		DWORD errCode     = GetLastError();
		char  errMsg[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 0, errMsg, 256, 0);
		TT_PANIC("Error retrieving temporary directory (error %d: '%s').", errCode, errMsg);
		return std::string();
	}
	
	return tmp;
#endif
}


std::string WindowsFileSystem::getSaveRootDir()
{
#if defined(_XBOX)
	// FIXME: Provide real (working) path for Xbox 360 save files
	TT_PANIC("Save root dir not correctly implemented for Xbox 360 yet.");
	return std::string();
#else
	// Use future-proof location for save data (guaranteed to be read/write accessible for any user)
	std::string saveDir;
	{
		// Get local appdata path
		char rawSaveDir[MAX_PATH] = { 0 };
		HRESULT hr = SHGetFolderPathA(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, rawSaveDir);
		if (SUCCEEDED(hr) == false)
		{
			TT_PANIC("Retrieving save data path failed (0x%08X).", hr);
			return std::string();
		}
		
		saveDir = rawSaveDir;
	}
	
	// Ensure the path has a trailing backslash
	if (*saveDir.rbegin() != '\\') saveDir += "\\";
	
	//TT_ASSERTMSG(dirExists(saveDir), "Local application data directory must exist.");
	
	// Append company and app name (client code is responsible for creating the directories)
	saveDir += "Two Tribes\\" + sanitizeFilename(m_appName) + "\\";
	
	return saveDir;
#endif
}


// Path functions

std::string WindowsFileSystem::getAbsolutePath(const std::string& p_path)
{
#if defined(_XBOX)
	TT_PANIC("Cannot get absolute path of file on Xbox 360.");
	return std::string();
#else
	char path[MAX_PATH] = { 0 };
	if (GetFullPathNameA(p_path.c_str(), MAX_PATH, path, NULL) == 0)
	{
		TT_PANIC("Unable to get full path of '%s' (err: %d).", p_path.c_str(), GetLastError());
		return std::string();
	}
	return std::string(path);
#endif
}


std::string WindowsFileSystem::getRelativePath(const std::string& p_path)
{
	return getRelativePathTo(p_path, getWorkingDir());
}


std::string WindowsFileSystem::getRelativePathTo(const std::string& p_path, const std::string& p_dir)
{
#if defined(_XBOX)
	TT_PANIC("Cannot get relative path of file on Xbox 360.");
	return std::string();
#else
	std::string cwd = getAbsolutePath(p_dir);
	if (cwd.empty())
	{
		TT_PANIC("Unable to get directory.");
		return std::string();
	}
	if (cwd.at(cwd.length() - 1) != '\\')
	{
		cwd += '\\';
	}
	std::string abs = getAbsolutePath(p_path);
	if (abs.empty())
	{
		TT_PANIC("Unable to get absolute path.");
		return std::string();
	}
	
	// check whether cwd or abs is a network dir/file
	bool cwdnetwork = cwd.size() >= 2 && cwd.at(0) == '\\' && cwd.at(1) == '\\';
	bool absnetwork = abs.size() >= 2 && abs.at(0) == '\\' && abs.at(1) == '\\';
	if (cwdnetwork != absnetwork)
	{
		// one of the two paths is a network path
		return std::string();
	}
	if (cwdnetwork)
	{
		// network path, server and share must match
		std::string::size_type cwdserverpos = cwd.find('\\', 2);
		if (cwdserverpos == std::string::npos)
		{
			TT_PANIC("Current working directory '%s' is an invalid network name.", cwd.c_str());
			return std::string();
		}
		std::string cwdserver = cwd.substr(2, cwdserverpos - 2);
		
		std::string::size_type cwdsharepos = cwd.find('\\', cwdserverpos + 1);
		if (cwdsharepos == std::string::npos)
		{
			TT_PANIC("Current working directory '%s' is an invalid network name.", cwd.c_str());
			return std::string();
		}
		std::string cwdshare = cwd.substr(cwdserverpos + 1, cwdsharepos - (cwdserverpos + 1));
		
		std::string::size_type absserverpos = abs.find('\\', 2);
		if (absserverpos == std::string::npos)
		{
			TT_PANIC("Path '%s' is an invalid network name.", abs.c_str());
			return std::string();
		}
		std::string absserver = abs.substr(2, absserverpos - 2);
		
		std::string::size_type abssharepos = abs.find('\\', absserverpos + 1);
		if (abssharepos == std::string::npos)
		{
			TT_PANIC("Path '%s' is an invalid network name.", abs.c_str());
			return std::string();
		}
		std::string absshare = abs.substr(absserverpos + 1, abssharepos - (absserverpos + 1));
		
		if (cwdserver != absserver)
		{
			// server mismatch
			return std::string();
		}
		
		if (cwdshare != absshare)
		{
			// share mismatch
			return std::string();
		}
	}
	
	// get common part of both strings
	typedef std::pair<std::string::iterator, std::string::iterator> itpair;
	if (cwd.size() > abs.size())
	{
		itpair common = std::mismatch(abs.begin(), abs.end(), cwd.begin());
		
		// search back until first backslash
		while (common.first != abs.begin())
		{
			if (*(common.first - 1) == '\\')
			{
				break;
			}
			--common.first;
			--common.second;
		}
		if (common.first == abs.begin())
		{
			// full mismatch
			return std::string();
		}
		
		abs = std::string(common.first, abs.end());
		cwd = std::string(common.second, cwd.end());
	}
	else
	{
		itpair common = std::mismatch(cwd.begin(), cwd.end(), abs.begin());
		
		// search back until first backslash
		while (common.first != cwd.begin())
		{
			if (*(common.first - 1) == '\\')
			{
				break;
			}
			--common.first;
			--common.second;
		}
		if (common.first == cwd.begin())
		{
			// full mismatch
			return std::string();
		}
		
		cwd = std::string(common.first, cwd.end());
		abs = std::string(common.second, abs.end());
	}
	
	while (cwd.empty() == false)
	{
		// for every directory remaining in cwd, add "..\" to abs
		std::string::size_type pos = cwd.find('\\');
		
		// current working dir always ends with backslash
		abs = "..\\" + abs;
		cwd = cwd.substr(pos + 1);
	}
	
	return abs;
#endif
}


std::string::value_type WindowsFileSystem::getDirSeparator() const
{
	return std::string::value_type('\\');
}


std::string WindowsFileSystem::getApplicationPath()
{
#if defined(_XBOX)
	TT_PANIC("Not implemented on Xbox 360.");
	return std::string();
#else
	char appname[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(0, appname, MAX_PATH) == 0)
	{
		DWORD errCode     = GetLastError();
		char  errMsg[256] = { 0 };
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, errCode, 0, errMsg, 256, 0);
		TT_PANIC("Error retrieving application path (error %d: '%s').", errCode, errMsg);
		return std::string();
	}
	
	return appname;
#endif
}


std::string WindowsFileSystem::sanitizeFilename(const std::string& p_name)
{
	if (p_name.empty())
	{
		TT_PANIC("Empty strings are not valid Windows file/directory names.");
		return std::string();
	}
	
	// Turn the specified name into a valid file/directory name, if it is not already one
	// The code below uses the file/directory naming rules from:
	// http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
	std::string sanitizedName(p_name);
	
	// Replace reserved characters with an underscore
	tt::str::replace(sanitizedName, "<",  "_");
	tt::str::replace(sanitizedName, ">",  "_");
	tt::str::replace(sanitizedName, ":",  "_");
	tt::str::replace(sanitizedName, "\"", "_");
	tt::str::replace(sanitizedName, "/",  "_");
	tt::str::replace(sanitizedName, "\\", "_");
	tt::str::replace(sanitizedName, "|",  "_");
	tt::str::replace(sanitizedName, "?",  "_");
	tt::str::replace(sanitizedName, "*",  "_");
	
	// FIXME: Strip leading spaces?
	
	if (sanitizedName.empty())
	{
		TT_PANIC("Transforming name '%s' into a valid Windows file/directory name "
		         "results in an empty name!", p_name.c_str());
		return std::string();
	}
	
	while (*sanitizedName.rbegin() == ' ' || *sanitizedName.rbegin() == '.')
	{
		// Remove trailing spaces
		sanitizedName = tt::str::trim(sanitizedName);
		if (sanitizedName.empty())
		{
			TT_PANIC("Transforming name '%s' into a valid Windows file/directory name "
			         "results in an empty name!", p_name.c_str());
			return std::string();
		}
		
		// Name should not end in a period (replace with an underscore)
		if (*sanitizedName.rbegin() == '.')
		{
			*sanitizedName.rbegin() = '_';
		}
	}
	
	return sanitizedName;
}


#if !defined(TT_BUILD_FINAL) && !defined(_XBOX)
bool WindowsFileSystem::validatePathCasing(const std::string& p_path, identifier p_fsID)
{
	// FIXME: compactPath does not work on paths that start with '../'; need a better method of checking
	std::string nativePath(fs::utils::compactPath(p_path, "/", p_fsID));
	const std::string actualPath(getActualPathName(nativePath));
	
	/*
	// Ignore trailing directory separators (in case of directory paths)
	while (nativePath.empty() == false && *nativePath.rbegin() == '\\')
	{
		nativePath.erase(nativePath.length() - 1);
	}
	//*/
	
	TT_ASSERTMSG(nativePath == actualPath,
	             "Filename casing of path '%s' does not match the on-disk casing '%s'. "
	             "This will cause problems on case-sensitive file systems.",
	             nativePath.c_str(), actualPath.c_str());
	return nativePath == actualPath;
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

WindowsFileSystem::WindowsFileSystem(fs::identifier p_id, const std::string& p_appName)
:
FileSystem(p_id),
m_appName(p_appName)
{
}

// Namespace end
}
}
