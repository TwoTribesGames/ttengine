#include <tt/fs/Dir.h>
#include <tt/fs/File.h>
#include <tt/fs/FileSystem.h>
#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


namespace tt {
namespace fs {

//--------------------------------------------------------------------------------------------------
// Public member functions

FileSystem::FileSystem(fs::identifier p_identifier)
:
m_identifier(p_identifier),
m_registered(false)
{
}


FileSystem::~FileSystem()
{
	if (m_registered)
	{
		fs::unregisterFileSystem(m_identifier);
	}
}


// Feature support check functions

bool FileSystem::supportsSaving()
{
	return false;
}


bool FileSystem::supportsAsync()
{
	return false;
}


bool FileSystem::supportsDirectories()
{
	return false;
}


bool FileSystem::supportsCommitting()
{
	return false;
}


// Commit functions

bool FileSystem::mountSaveVolume(OpenMode /*p_mode*/)
{
	// Default implementation assumes no separate mounting of save volume is necessary, so always succeeds
	return true;
}


bool FileSystem::unmountSaveVolume()
{
	// Default implementation assumes no separate mounting of save volume is necessary, so always succeeds
	return true;
}


bool FileSystem::isSaveVolumeMounted()
{
	// Default implementation assumes no separate mounting of save volume is necessary, so always mounted
	return true;
}


bool FileSystem::hasChanges()
{
	return false;
}


bool FileSystem::canCommit()
{
	return true;
}


bool FileSystem::commit()
{
	return true;
}


// Basic file functions

size_type FileSystem::readAsync(const FilePtr& p_file,
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


size_type FileSystem::writeAsync(const FilePtr& p_file,
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


bool FileSystem::flush(const FilePtr&)
{
	return true;
}


// Content functions

code::BufferPtr FileSystem::getFileContent(const std::string& p_path)
{
	FilePtr file = fs::open(p_path, fs::OpenMode_Read, getIdentifier());
	if (file == 0)
	{
		TT_PANIC("Unable to open %s.", p_path.c_str());
		return code::BufferPtr();
	}
	
	return getFileContent(file);
}


code::BufferPtr FileSystem::getFileContent(const FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return code::BufferPtr();
	}
	
	const size_type fileSize = p_file->getLength();
	
	code::BufferPtrForCreator content(new code::Buffer(static_cast<code::Buffer::size_type>(fileSize)));
	if (content->getData() == 0)
	{
		return code::BufferPtr();
	}
	
	const pos_type pos = p_file->getPosition();
	p_file->seekToBegin();
	
	const size_type bytesRead = p_file->read(content->getData(), fileSize);
	content->setSize(static_cast<code::Buffer::size_type>(bytesRead));
	
	p_file->seek(static_cast<pos_type>(pos), SeekPos_Set);
	
	return content;
}


// Asynchronous status functions

bool FileSystem::isBusy(const FilePtr&)
{
	return false;
}


bool FileSystem::isSucceeded(const FilePtr&)
{
	return true;
}


bool FileSystem::wait(const FilePtr&)
{
	return true;
}


bool FileSystem::cancel(const FilePtr&)
{
	return true;
}


// Position / size functions

bool FileSystem::seekToBegin(const FilePtr& p_file)
{
	return seek(p_file, 0, SeekPos_Set);
}


bool FileSystem::seekToEnd(const FilePtr& p_file)
{
	return seek(p_file, 0, SeekPos_End);
}


// Time functions

time_type FileSystem::getCreationTime(const FilePtr&)
{
	return time_type();
}


time_type FileSystem::getAccessTime(const FilePtr&)
{
	return time_type();
}


time_type FileSystem::getWriteTime(const FilePtr&)
{
	return time_type();
}


bool FileSystem::setWriteTime(const FilePtr&, time_type)
{
	TT_PANIC("This filesystem implementation does not have setWriteTime support.");
	return false;
}


s64 FileSystem::convertToUnixTime(time_type p_nativeTime)
{
	return static_cast<s64>(p_nativeTime);
}


time_type FileSystem::convertToNativeTime(s64 p_unixTime)
{
	return static_cast<time_type>(p_unixTime);
}


// Directory functions

bool FileSystem::openDir(DirPtr&, const std::string&, const std::string&)
{
	TT_PANIC("This filesystem implementation does not have directory support.");
	return false;
}


bool FileSystem::closeDir(Dir*)
{
	return false;
}


bool FileSystem::readDir(const DirPtr&, DirEntry&)
{
	TT_PANIC("This filesystem implementation does not have directory support.");
	return false;
}


// Create / Destroy functions

bool FileSystem::destroyFile(const std::string&)
{
	return false;
}


bool FileSystem::copyFile(const std::string&, const std::string&, bool)
{
	return false;
}


bool FileSystem::moveFile(const std::string&, const std::string&, bool)
{
	return false;
}


bool FileSystem::dirExists(const std::string&)
{
	TT_PANIC("This filesystem implementation does not have directory support.");
	return false;
}


bool FileSystem::createDir(const std::string&)
{
	TT_PANIC("This filesystem implementation does not have directory support.");
	return false;
}


bool FileSystem::createSaveRootDir()
{
	// Default behavior: if this file system has a non-empty save root path, attempt to create it.
	// Otherwise, assume no extra handling is required for the save data root dir
	const std::string saveRoot(getSaveRootDir());
	if (saveRoot.empty() == false && dirExists(saveRoot) == false)
	{
		return utils::createDirRecursive(saveRoot, getIdentifier());
	}
	
	return true;
}


bool FileSystem::destroyDir(const std::string&)
{
	TT_PANIC("This filesystem implementation does not have directory support.");
	return false;
}


// Working Directory functions

std::string FileSystem::getWorkingDir()
{
	return std::string();
}


bool FileSystem::setWorkingDir(const std::string&)
{
	return false;
}


std::string FileSystem::getApplicationDir()
{
	return std::string();
}


std::string FileSystem::getTemporaryDir()
{
	return std::string();
}


std::string FileSystem::getSaveRootDir()
{
	return std::string();
}


// Path functions

std::string FileSystem::getAbsolutePath(const std::string&)
{
	return std::string();
}


std::string FileSystem::getRelativePath(const std::string&)
{
	return std::string();
}


std::string FileSystem::getRelativePathTo(const std::string&, const std::string&)
{
	return std::string();
}


std::string::value_type FileSystem::getDirSeparator() const
{
	return std::string::value_type('/');
}


std::string FileSystem::getApplicationPath()
{
	return std::string();
}


// Misc functions

s32 FileSystem::getAllocationType() const
{
	return 0;
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

bool FileSystem::validate(const FilePtr& p_file) const
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
	
	if (p_file->getFileSystem() != getIdentifier())
	{
		TT_PANIC("File was not opened with this file system. Cannot operate on it.\n"
		         "This file system ID: %d.\nFile system ID used to open the file: %d.",
		         getIdentifier(), p_file->getFileSystem());
		return false;
	}
	
	return true;
}


bool FileSystem::validate(const DirPtr& p_dir) const
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
	
	/* FIXME: Cannot perform this sanity check, because of PassThroughFileSystem:
	//        tt::fs::openDir creates a Dir for the PassThroughFileSystem ID,
	//        it forwards the call to source FileSystem: IDs do not match.
	if (p_dir->getFileSystem() != getIdentifier())
	{
		TT_PANIC("Directory was not opened with this file system. Cannot operate on it.\n"
		         "This file system ID: %d.\nFile system ID used to open the directory: %d.",
		         getIdentifier(), p_file->getFileSystem());
		return false;
	}
	// */
	
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// namespace end
}
}
