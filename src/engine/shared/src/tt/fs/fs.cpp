#include <map>
#include <set>

#include <tt/fs/Dir.h>
#include <tt/fs/File.h>
#include <tt/fs/FileSystem.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/mem/util.inl>


namespace tt {
namespace fs {

typedef std::map<identifier, FileSystem*> FileSystems;
typedef std::set<identifier> FileSystemIDs;

static FileSystems   ms_fileSystems; // FIXME: Incorrect prefix; should be g_.
static FileSystemIDs ms_usedIDs; // File system IDs that have ever been used during the lifetime of this app. // FIXME: Incorrect prefix; should be g_.


// Filesystem functions

bool registerFileSystem(FileSystem* p_fileSystem, identifier p_identifier)
{
	if (p_fileSystem == 0)
	{
		TT_PANIC("No filesystem specified.");
		return false;
	}
	
	if (ms_fileSystems.find(p_identifier) != ms_fileSystems.end())
	{
		TT_PANIC("Filesystem with identifier %d already registered.", p_identifier);
		return false;
	}
	
	/*
	TT_ASSERTMSG(ms_usedIDs.find(p_identifier) == ms_usedIDs.end(),
	             "A file system with identifier %d has been registered (and unregistered) before "
	             "during the lifetime of this application. This usage is discouraged.",
	             p_identifier);
	// */
	
	ms_fileSystems.insert(FileSystems::value_type(p_identifier, p_fileSystem));
	ms_usedIDs.insert(p_identifier);
	p_fileSystem->m_registered = true;
	return true;
}


bool unregisterFileSystem(identifier p_identifier)
{
	FileSystems::iterator it = ms_fileSystems.find(p_identifier);
	
	if (it == ms_fileSystems.end())
	{
		TT_PANIC("No filesystem with identifier %d registered.", p_identifier);
		return false;
	}
	
	ms_fileSystems.erase(it);
	return true;
}


FileSystem* getFileSystem(identifier p_identifier)
{
	FileSystems::iterator it = ms_fileSystems.find(p_identifier);
	if (it == ms_fileSystems.end())
	{
		TT_PANIC("No filesystem with identifier %d registered.", p_identifier);
		return 0;
	}
	return (*it).second;
}


// Feature support check functions

bool supportsSaving(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->supportsSaving();
}


bool supportsAsync(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->supportsAsync();
}


bool supportsDirectories(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->supportsDirectories();
}


bool supportsCommitting(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->supportsCommitting();
}


// Commit functions

bool mountSaveVolume(OpenMode p_mode, identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->mountSaveVolume(p_mode);
}


bool unmountSaveVolume(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->unmountSaveVolume();
}


bool isSaveVolumeMounted(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->isSaveVolumeMounted();
}


bool hasChanges(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->hasChanges();
}


bool canCommit(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->canCommit();
}


bool commit(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->commit();
}


// File functions

FilePtr open(const std::string& p_path, OpenMode p_mode, identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	if (impl == 0) return FilePtr();
	
	FilePtr ret(new File(p_path, p_identifier), File::deleteFile);
	ret->m_this = ret;
	
	if (impl->open(ret, p_path, p_mode) == false)
	{
		// NOTE: We expect the underlying file system to have triggered a panic about this situation
		//       (because it can provide a more detailed error message).
		//TT_PANIC("Failed to open '%s'.", p_path.c_str());
		return FilePtr();
	}
	
	return ret;
}


bool close(File* p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->close(p_file);
}


size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return size_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? size_type() : impl->read(p_file, p_buffer, p_length);
}


size_type readAsync(const FilePtr& p_file,
                    void*          p_buffer,
                    size_type      p_length,
                    callback       p_callback,
                    void*          p_arg)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return size_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? size_type() : impl->readAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return size_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? size_type() : impl->write(p_file, p_buffer, p_length);
}


size_type writeAsync(const FilePtr& p_file,
                     const void*    p_buffer,
                     size_type      p_length,
                     callback       p_callback,
                     void*          p_arg)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return size_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? size_type() : impl->writeAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


bool flush(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->flush(p_file);
}


// Content functions

code::BufferPtr getFileContent(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No file specified.");
		return code::BufferPtr();
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? code::BufferPtr() : impl->getFileContent(p_path);
}


code::BufferPtr getFileContent(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return code::BufferPtr();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? code::BufferPtr() : impl->getFileContent(p_file);
}


// Asynchronous status functions

bool isBusy(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->isBusy(p_file);
}


bool isSucceeded(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->isSucceeded(p_file);
}


bool wait(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->wait(p_file);
}


bool cancel(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->cancel(p_file);
}


// Position / size functions

bool seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->seek(p_file, p_offset, p_position);
}


bool seekToBegin(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->seekToBegin(p_file);
}


bool seekToEnd(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->seekToEnd(p_file);
}


pos_type getPosition(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return pos_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? pos_type() : impl->getPosition(p_file);
}


size_type getLength(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return size_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? size_type() : impl->getLength(p_file);
}


// Time functions

time_type getCreationTime(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return time_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? time_type() : impl->getCreationTime(p_file);
}


time_type getAccessTime(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return time_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? time_type() : impl->getAccessTime(p_file);
}


time_type getWriteTime(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return time_type();
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->getWriteTime(p_file);
}


bool setWriteTime(const FilePtr& p_file, time_type p_time)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_file->getFileSystem());
	return (impl == 0) ? false : impl->setWriteTime(p_file, p_time);
}


s64 convertToUnixTime(time_type p_nativeTime, identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? 0 : impl->convertToUnixTime(p_nativeTime);
}


time_type convertToNativeTime(s64 p_unixTime, identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? 0 : impl->convertToNativeTime(p_unixTime);
}


// Directory functions

DirPtr openDir(const std::string& p_path,
               const std::string& p_filter,
               identifier         p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	if (impl == 0) return DirPtr();
	
	DirPtr ret(new Dir(p_path, p_identifier), Dir::deleteDir);
	ret->m_this = ret;
	
	if (impl->openDir(ret, p_path, p_filter) == false)
	{
		TT_PANIC("Failed to open '%s'.", p_path.c_str());
		return DirPtr();
	}
	
	return ret;
}


bool closeDir(Dir* p_dir)
{
	if (p_dir == 0)
	{
		TT_PANIC("No directory specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_dir->getFileSystem());
	return (impl == 0) ? false : impl->closeDir(p_dir);
}


bool readDir(const DirPtr& p_dir, DirEntry& p_entry)
{
	if (p_dir == 0)
	{
		TT_PANIC("No directory specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_dir->getFileSystem());
	return (impl == 0) ? false : impl->readDir(p_dir, p_entry);
}


// Create / Destroy functions

bool fileExists(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->fileExists(p_path);
}


bool destroyFile(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->destroyFile(p_path);
}


bool copyFile(const std::string& p_existingFile,
              const std::string& p_newFile,
              bool               p_failIfExists,
              identifier         p_identifier)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No input file specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No output file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->copyFile(p_existingFile, p_newFile, p_failIfExists);
}



bool moveFile(const std::string& p_existingFile,
              const std::string& p_newFile,
              bool               p_failIfExists,
              identifier         p_identifier)
{
	if (p_existingFile.empty())
	{
		TT_PANIC("No input file specified.");
		return false;
	}
	
	if (p_newFile.empty())
	{
		TT_PANIC("No output file specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->moveFile(p_existingFile, p_newFile, p_failIfExists);
}


bool dirExists(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->dirExists(p_path);
}


bool createDir(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No directory specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->createDir(p_path);
}


bool createSaveRootDir(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->createSaveRootDir();
}


bool destroyDir(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No directory specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->destroyDir(p_path);
}


// Working Directory functions


std::string getWorkingDir(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getWorkingDir();
}


bool setWorkingDir(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? false : impl->setWorkingDir(p_path);
}


std::string getApplicationDir(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getApplicationDir();
}


std::string getTemporaryDir(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getTemporaryDir();
}


std::string getSaveRootDir(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getSaveRootDir();
}


// Path functions

std::string getAbsolutePath(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return std::string();
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getAbsolutePath(p_path);
}


std::string getRelativePath(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return std::string();
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getRelativePath(p_path);
}


std::string getRelativePathTo(const std::string& p_path, const std::string& p_dir, identifier p_identifier)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return std::string();
	}
	
	if (p_dir.empty())
	{
		TT_PANIC("No directory specified.");
		return std::string();
	}
	
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getRelativePathTo(p_path, p_dir);
}


std::string::value_type getDirSeparator(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string::value_type('/') : impl->getDirSeparator();
}


std::string getApplicationPath(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? std::string() : impl->getApplicationPath();
}


// Misc functions

s32 getAllocationType(identifier p_identifier)
{
	FileSystem* impl = getFileSystem(p_identifier);
	return (impl == 0) ? 0 : impl->getAllocationType();
}


// Helper functions

bool isUpToDate(const std::string& p_target, const std::string& p_prerequisite, identifier p_identifier)
{
	if (p_target.empty())
	{
		TT_PANIC("No target specified.");
		return false;
	}
	
	if (p_prerequisite.empty())
	{
		TT_PANIC("No prerequisite specified.");
		return false;
	}
	
	if (getFileSystem(p_identifier) == 0)
	{
		return false;
	}
	
	time_type targettime = 0;
	if (fileExists(p_target, p_identifier) == false)
	{
		return false;
	}
	else
	{
		FilePtr target = open(p_target, OpenMode_Read, p_identifier);
		if (target == 0)
		{
			return false;
		}
		targettime = target->getWriteTime();
	}
	
	time_type prerequisitetime = 0;
	if (fileExists(p_prerequisite, p_identifier) == false)
	{
		return false;
	}
	else
	{
		FilePtr prerequisite = open(p_prerequisite, OpenMode_Read, p_identifier);
		if (prerequisite == 0)
		{
			return false;
		}
		prerequisitetime = prerequisite->getWriteTime();
	}
	return targettime >= prerequisitetime;
}


bool readNarrowString(const FilePtr& p_file, std::string* p_value)
{
	s32 length = 0;
	if (readInteger(p_file, &length) == false)
	{
		return false;
	}
	
	if (length == 0)
	{
		// Empty (zero length) string; nothing further to read
		p_value->clear();
		return true;
	}
	
	if(length > p_file->getLength())
	{
		// String length is larger than file size; cannot be valid
		return false;
	}
	
	code::Buffer data(length);
	size_type readData = read(p_file, data.getData(), length);
	if (readData != length)
	{
		return false;
	}
	
	p_value->assign(static_cast<const char*>(data.getData()),
	                static_cast<std::string::size_type>(length));
	
	return true;
}


bool writeNarrowString(const FilePtr& p_file, const std::string& p_string)
{
	s32 length = static_cast<s32>(p_string.length());
	if (writeInteger(p_file, length) == false)
	{
		return false;
	}
	
	if (length == 0)
	{
		// Empty string; nothing further to write
		return true;
	}
	
	size_type writtenData = write(p_file, p_string.c_str(), length);
	return writtenData == length;
}


bool readWideString(const FilePtr& p_file, std::wstring* p_value)
{
	u16 length = 0;
	if (readInteger(p_file, &length) == false)
	{
		return false;
	}
	length /= 2;
	
	if (length == 0)
	{
		p_value->clear();
		return true;
	}
	
	if (length > p_file->getLength())
	{
		return false;
	}
	
	for (u16 i = 0; i < length; ++i)
	{
		u16 temp = 0;
		if (readInteger(p_file, &temp) == false)
		{
			p_value->clear();
			return false;
		}
		*p_value += static_cast<std::wstring::value_type>(temp);
	}
	
	return true;
}


bool writeWideString(const FilePtr& p_file, const std::wstring& p_string)
{
	u16 length = static_cast<u16>(p_string.length() * 2);
	if (writeInteger(p_file, length) == false)
	{
		return false;
	}
	
	for (std::wstring::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		if (writeInteger(p_file, static_cast<u16>(*it)) == false)
		{
			return false;
		}
	}
	
	return true;
}


bool readReal(const FilePtr& p_file, real* p_value)
{
	u32 value = 0;
	bool retValue = readInteger(p_file, &value);
	
	float readFloat = 0.0f;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&readFloat, &value, sizeof(float));
	*p_value = real(readFloat);
	
	return retValue;
}


bool writeReal(const FilePtr& p_file, real p_value)
{
	float writeFloat = realToFloat(p_value);
	u32 value = 0;
	
	TT_STATIC_ASSERT(sizeof(u32) == sizeof(float));
	mem::copy8(&value, &writeFloat, sizeof(u32));
	
	return writeInteger(p_file, value);
}


bool readReal(const FilePtr& p_file, real64* p_value)
{
	u64 value = 0;
	bool retValue = readInteger(p_file, &value);
	
	real64 readValue = 0.0f;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	mem::copy8(&readValue, &value, sizeof(real64));
	*p_value = real64(readValue);
	
	return retValue;
}


bool writeReal(const FilePtr& p_file, real64 p_value)
{
	real64 writeValue = realToFloat(p_value);
	u64 value = 0;
	
	TT_STATIC_ASSERT(sizeof(u64) == sizeof(real64));
	mem::copy8(&value, &writeValue, sizeof(u64));
	
	return writeInteger(p_file, value);
}

// Namespace end
}
}
