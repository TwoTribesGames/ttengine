#include <algorithm>
#include <cstdio>

#include <tt/fs/File.h>
#include <tt/fs/PassThroughFileSystem.h>
#include <tt/mem/util.h>


namespace tt {
namespace fs {

//--------------------------------------------------------------------------------------------------
// Public member functions

// Feature support check functions

bool PassThroughFileSystem::supportsSaving()
{
	return fs::supportsSaving(m_source);
}


bool PassThroughFileSystem::supportsAsync()
{
	return fs::supportsAsync(m_source);
}


bool PassThroughFileSystem::supportsDirectories()
{
	return fs::supportsDirectories(m_source);
}


bool PassThroughFileSystem::supportsCommitting()
{
	return fs::supportsCommitting(m_source);
}


// Commit functions

bool PassThroughFileSystem::mountSaveVolume(OpenMode p_mode)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->mountSaveVolume(p_mode);
}


bool PassThroughFileSystem::unmountSaveVolume()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->unmountSaveVolume();
}


bool PassThroughFileSystem::isSaveVolumeMounted()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->isSaveVolumeMounted();
}


bool PassThroughFileSystem::hasChanges()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->hasChanges();
}


bool PassThroughFileSystem::canCommit()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->canCommit();
}


bool PassThroughFileSystem::commit()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->commit();
}


// Basic file functions

bool PassThroughFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->open(p_file, p_path, p_mode);
}


bool PassThroughFileSystem::close(File* p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->close(p_file);
}


size_type PassThroughFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->read(p_file, p_buffer, p_length);
}


size_type PassThroughFileSystem::readAsync(const FilePtr& p_file,
                                           void*          p_buffer,
                                           size_type      p_length,
                                           callback       p_callback,
                                           void*          p_arg)
{
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->readAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


size_type PassThroughFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->write(p_file, p_buffer, p_length);
}


size_type PassThroughFileSystem::writeAsync(const FilePtr& p_file,
                                            const void*    p_buffer,
                                            size_type      p_length,
                                            callback       p_callback,
                                            void*          p_arg)
{
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->writeAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


bool PassThroughFileSystem::flush(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->flush(p_file);
}


// Content functions

code::BufferPtr PassThroughFileSystem::getFileContent(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? code::BufferPtr() : src->getFileContent(p_path);
}


code::BufferPtr PassThroughFileSystem::getFileContent(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? code::BufferPtr() : src->getFileContent(p_file);
}


// Asynchronous status functions

bool PassThroughFileSystem::isBusy(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->isBusy(p_file);
}


bool PassThroughFileSystem::isSucceeded(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->isSucceeded(p_file);
}


bool PassThroughFileSystem::wait(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->wait(p_file);
}


bool PassThroughFileSystem::cancel(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->cancel(p_file);
}


// Position / size functions

bool PassThroughFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->seek(p_file, p_offset, p_position);
}


bool PassThroughFileSystem::seekToBegin(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->seekToBegin(p_file);
}


bool PassThroughFileSystem::seekToEnd(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->seekToEnd(p_file);
}


pos_type PassThroughFileSystem::getPosition(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? pos_type() : src->getPosition(p_file);
}


size_type PassThroughFileSystem::getLength(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? size_type() : src->getLength(p_file);
}


// Time functions

time_type PassThroughFileSystem::getCreationTime(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? time_type() : src->getCreationTime(p_file);
}


time_type PassThroughFileSystem::getAccessTime(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? time_type() : src->getAccessTime(p_file);
}


time_type PassThroughFileSystem::getWriteTime(const FilePtr& p_file)
{
	FileSystem* src = getSource();
	return (src == 0) ? time_type() : src->getWriteTime(p_file);
}


bool PassThroughFileSystem::setWriteTime(const FilePtr& p_file, time_type p_time)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->setWriteTime(p_file, p_time);
}


s64 PassThroughFileSystem::convertToUnixTime(time_type p_nativeTime)
{
	FileSystem* src = getSource();
	return (src == 0) ? 0 : src->convertToUnixTime(p_nativeTime);
}


time_type PassThroughFileSystem::convertToNativeTime(s64 p_unixTime)
{
	FileSystem* src = getSource();
	return (src == 0) ? 0 : src->convertToNativeTime(p_unixTime);
}


// Directory functions

bool PassThroughFileSystem::openDir(DirPtr&            p_dir,
                                    const std::string& p_path,
                                    const std::string& p_filter)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->openDir(p_dir, p_path, p_filter);
}


bool PassThroughFileSystem::closeDir(Dir* p_dir)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->closeDir(p_dir);
}


bool PassThroughFileSystem::readDir(const DirPtr& p_dir, DirEntry& p_entry)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->readDir(p_dir, p_entry);
}


// Create / Destroy functions

bool PassThroughFileSystem::fileExists(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->fileExists(p_path);
}


bool PassThroughFileSystem::destroyFile(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->destroyFile(p_path);
}


bool PassThroughFileSystem::copyFile(const std::string& p_existingFile,
                                     const std::string& p_newFile,
                                     bool               p_failIfExists)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->copyFile(p_existingFile, p_newFile, p_failIfExists);
}


bool PassThroughFileSystem::moveFile(const std::string& p_existingFile,
                                     const std::string& p_newFile,
                                     bool               p_failIfExists)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->moveFile(p_existingFile, p_newFile, p_failIfExists);
}


bool PassThroughFileSystem::dirExists(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->dirExists(p_path);
}


bool PassThroughFileSystem::createDir(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->createDir(p_path);
}


bool PassThroughFileSystem::createSaveRootDir()
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->createSaveRootDir();
}


bool PassThroughFileSystem::destroyDir(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->destroyDir(p_path);
}


// Working Directory functions

std::string PassThroughFileSystem::getWorkingDir()
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getWorkingDir();
}


bool PassThroughFileSystem::setWorkingDir(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? false : src->setWorkingDir(p_path);
}


std::string PassThroughFileSystem::getApplicationDir()
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getApplicationDir();
}


std::string PassThroughFileSystem::getTemporaryDir()
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getTemporaryDir();
}


std::string PassThroughFileSystem::getSaveRootDir()
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getSaveRootDir();
}


// Path functions

std::string PassThroughFileSystem::getAbsolutePath(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getAbsolutePath(p_path);
}


std::string PassThroughFileSystem::getRelativePath(const std::string& p_path)
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getRelativePath(p_path);
}


std::string PassThroughFileSystem::getRelativePathTo(const std::string& p_path, const std::string& p_dir)
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getRelativePathTo(p_path, p_dir);
}


std::string::value_type PassThroughFileSystem::getDirSeparator() const
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string::value_type() : src->getDirSeparator();
}


std::string PassThroughFileSystem::getApplicationPath()
{
	FileSystem* src = getSource();
	return (src == 0) ? std::string() : src->getApplicationPath();
}


// Misc functions

s32 PassThroughFileSystem::getAllocationType() const
{
	FileSystem* src = getSource();
	return (src == 0) ? 0 : src->getAllocationType();
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

PassThroughFileSystem::PassThroughFileSystem(identifier p_id, identifier p_source)
:
FileSystem(p_id),
m_source(p_source)
{
}


FileSystem* PassThroughFileSystem::getSource() const
{
	return getFileSystem(m_source);
}

// Namespace end
}
}
